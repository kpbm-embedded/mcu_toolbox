# shellcheck shell=bash
# update.sh - plugin-side update API: version lookups, verified downloads, side-by-side installs,
# rollback state and the default sdk_rollback / sdk_prune (see ../PLUGIN.md "Update / rollback contract").
#
# Sourced inside every plugin subshell before <sdk>/sdk.sh (formerly host/update-lib.sh; that path is now a shim).
# Needs bash, coreutils, curl, tar and python3 (the host CPython from host/env.sh).
# Nothing here deletes: old versions stay on disk, prune moves them to the Trash.

# shellcheck disable=SC2034  # read and set by the plugins' sdk_update
UPD_RC=0

# True when updates may be applied (MCU_YES=1 and not --dry-run).
upd_yes() {
  [[ "${MCU_YES:-0}" == 1 && "${MCU_DRY_RUN:-0}" != 1 ]]
}

# curl with retries and timeouts; UPD_TIMEOUT bounds the whole transfer (default 30 min).
upd_curl() {
  curl -fsSL --retry 3 --connect-timeout 15 -m "${UPD_TIMEOUT:-1800}" "$@"
}

# ---------------------------------------------------------------- version lookups

# GitHub REST call; the token (GH_TOKEN, GITHUB_TOKEN or `gh auth token`) goes through a curl config on stdin, never argv.
upd_gh_api() {
  local path="$1" token="${GH_TOKEN:-${GITHUB_TOKEN:-}}"
  [[ -n "$token" ]] || token="$(gh auth token 2>/dev/null || true)"
  if [[ -n "$token" ]]; then
    printf 'header = "Authorization: Bearer %s"\n' "$token" | curl -fsS -m 30 -K - "https://api.github.com/$path"
  else
    curl -fsS -m 30 "https://api.github.com/$path"
  fi
}

# Newest published (non-draft, non-prerelease) release tag of <owner/repo>.
upd_gh_latest() {
  local repo="$1"
  upd_gh_api "repos/$repo/releases?per_page=30" 2>/dev/null | python3 -I -c '
import json, sys
try:
    for release in json.load(sys.stdin):
        if not release["draft"] and not release["prerelease"]:
            print(release["tag_name"])
            break
except Exception:
    pass'
}

# upd_gh_asset <repo> <tag> <name-regex> -> "url<TAB>sha256-digest-or-empty<TAB>sha-asset-url-or-empty"
upd_gh_asset() {
  local repo="$1" tag="$2" pattern="$3"
  upd_gh_api "repos/$repo/releases/tags/$tag" | python3 -I -c '
import json, re, sys
release = json.load(sys.stdin)
pattern = sys.argv[1]
assets = release.get("assets", [])
for asset in assets:
    if not re.fullmatch(pattern, asset["name"]):
        continue
    sums = (asset["name"] + ".sha256", asset["name"] + ".sha", asset["name"] + ".sha256sum")
    sha_url = next((b["browser_download_url"] for b in assets if b["name"] in sums), "")
    digest = (asset.get("digest") or "").replace("sha256:", "")
    print(asset["browser_download_url"], digest, sha_url, sep="\t")
    break' "$pattern"
}

# upd_git_newest_tag <repo-url|dir> <ERE>: highest matching tag (case-insensitive version sort).
upd_git_newest_tag() {
  local repo="$1" pattern="$2"
  upd_git_tags "$repo" | grep -E "$pattern" | awk '{print tolower($0) "\t" $0}' | sort -V -k1,1 | tail -1 | cut -f2
}

# All tag names of a local clone or a remote URL.
upd_git_tags() {
  local repo="$1"
  if [[ -d "$repo" ]]; then
    git -C "$repo" tag
  else
    git ls-remote --tags --refs "$repo" | sed 's#.*refs/tags/##'
  fi
}

# Latest version of a PyPI package.
upd_pypi_latest() {
  local package="$1"
  upd_curl -m 30 "https://pypi.org/pypi/$package/json" 2>/dev/null | python3 -I -c '
import json, sys
try:
    print(json.load(sys.stdin)["info"]["version"])
except Exception:
    pass'
}

# ---------------------------------------------------------------- download + verify

# upd_fetch <url> <dest> [url:<sha-file-url> | hex:<sha256>]
# Downloads once (cached in dest), verifies sha256 and sets UPD_SHA. Without a published sum the computed
# one is recorded as "(unpublished)" in <sdk>/.update-sha256.
upd_fetch() {
  local url="$1" dest="$2" sha_source="${3:-}" want="" got
  mkdir -p "$(dirname "$dest")"
  if [[ ! -s "$dest" ]]; then
    upd_curl -o "$dest.part" "$url" || return 1
    mv "$dest.part" "$dest"
  fi
  got="$(sha256sum "$dest" | cut -d' ' -f1)"
  want="$(upd_expected_sha "$sha_source")"
  if [[ -z "$want" ]]; then
    echo "  sha256: project publishes none; computed $got (recorded)"
    UPD_SHA="$got (unpublished)"
  elif [[ "${want,,}" != "$got" ]]; then
    echo "  sha256 MISMATCH: want $want got $got (file kept as $dest.bad)" >&2
    mv "$dest" "$dest.bad"
    return 1
  else
    echo "  sha256 ok $got"
    UPD_SHA="$got"
  fi
  printf '%s\t%s\t%s\n' "$(date +%F)" "$(basename "$dest")" "$UPD_SHA" >>"${SDK_DIR:-.}/.update-sha256"
}

# Resolve "url:<sum file>" or "hex:<sha256>" to the expected hash ("" when none is published).
upd_expected_sha() {
  local spec="$1"
  case "$spec" in
    url:*) upd_curl -m 60 "${spec#url:}" | grep -oiE '[0-9a-f]{64}' | head -1 ;;
    hex:*) printf '%s\n' "${spec#hex:}" ;;
    *) printf '\n' ;;
  esac
}

# upd_unpack <archive> <dest-dir> [strip]: extract into <dest>.part, then rename; an existing dest is reused.
upd_unpack() {
  local archive="$1" dest="$2" strip="${3:-1}"
  if [[ -d "$dest" ]]; then
    echo "  $dest exists (kept, reused)"
    return 0
  fi
  mkdir -p "$dest.part" \
    && tar -xf "$archive" -C "$dest.part" --strip-components="$strip" \
    && mv -T "$dest.part" "$dest"
}

# ---------------------------------------------------------------- rollback state

# upd_rec <kind> <path-relative-to-SDK_DIR> <previous> [extra]; kinds: link git pip west repo arduino file.
upd_rec() {
  [[ -n "${UPD_STATE:-}" ]] || return 0
  printf '%s\t%s\t%s\t%s\n' "$1" "$2" "$3" "${4:-}" >>"$UPD_STATE"
}

# Remember the commit (and branch) a repo under SDK_DIR is on now.
upd_git_rec() {
  local dir="$1" head branch
  head="$(git -C "$SDK_DIR/$dir" rev-parse HEAD)"
  branch="$(git -C "$SDK_DIR/$dir" symbolic-ref -q --short HEAD || true)"
  upd_rec git "$dir" "$head" "$branch"
}

# upd_flip <stable-link abs path> <new target relative to the link's dir>: atomic symlink switch, old target recorded.
upd_flip() {
  local link="$1" new="$2" prev=""
  if [[ -L "$link" ]]; then
    prev="$(readlink "$link")"
  elif [[ -e "$link" ]]; then
    echo "  $link is a real directory, not a stable symlink: adopt it as $(dirname "$link")/<name>-<version> first" >&2
    return 1
  fi
  [[ "$prev" == "$new" ]] && return 0
  upd_rec link "${link#"$SDK_DIR"/}" "$prev" "$new"
  ln -sfn "$new" "$link.tmp$$" && mv -T "$link.tmp$$" "$link"
}

# Run patch-rpath on a freshly installed dir before it is switched on.
upd_patch() {
  ("$MCU_ROOT/host/patch-rpath.sh" "$@") 2>&1 | sed 's/^/  /' | tail -5 || true
}

# ---------------------------------------------------------------- reporting

# Append one row (sdk, component, old, new, result) to the run's results file.
upd_log() {
  [[ -n "${MCU_RESULTS:-}" ]] || return 0
  printf '%s\t%s\t%s\t%s\t%s\n' "${SDK:-?}" "$1" "$2" "$3" "$4" >>"$MCU_RESULTS"
}

# upd_report <component> <installed> <latest>: print and record one component (no install).
upd_report() {
  local component="$1" installed="$2" latest="${3:-}" status=current note=""
  if [[ -z "$latest" ]]; then
    printf '  %-24s %s -> ?   (latest unknown: network?)\n' "$component" "$installed"
    upd_log "$component" "$installed" "?" unknown
    return 0
  fi
  if [[ "$installed" != "$latest" ]]; then
    status=newer
    note='   (newer available)'
  fi
  printf '  %-24s %s -> %s%s\n' "$component" "$installed" "$latest" "$note"
  upd_log "$component" "$installed" "$latest" "$status"
}

# upd_step <component> <installed> <latest> <installer...>: report, and run the installer only when applying.
upd_step() {
  local component="$1" old="$2" new="$3"
  shift 3
  upd_report "$component" "$old" "$new"
  [[ -z "$new" || "$old" == "$new" ]] && return 0
  upd_yes || return 0
  if "$@"; then
    printf '  -> %s updated to %s\n' "$component" "$new"
    upd_log "$component" "$old" "$new" updated
  else
    echo "  FAILED: $component -> $new" >&2
    upd_log "$component" "$old" "$new" failed
    UPD_RC=1
  fi
}

# Remind the user how to apply after a dry run.
upd_dry_hint() {
  upd_yes || echo "  (dry run; 'mcu update ${SDK:-<sdk>} --yes' applies, verifies and rolls back on failure)"
}

# ---------------------------------------------------------------- xPack tools

# upd_xpack_info <repo> -> "tag<TAB>package-version<TAB>url<TAB>sha256" for the newest tag's linux-x64 binary.
# Location and sha256 come from package.json of that tag (".1" tags can be npm-only re-publishes).
upd_xpack_info() {
  local repo="$1" tag
  tag="$(upd_git_newest_tag "https://github.com/$repo" '^v[0-9]+(\.[0-9]+)*-[0-9]+(\.[0-9]+)?$')"
  [[ -n "$tag" ]] || return 1
  upd_curl -m 30 "https://raw.githubusercontent.com/$repo/$tag/package.json" 2>/dev/null | python3 -I -c '
import json, sys
package = json.load(sys.stdin)
binaries = package["xpack"]["binaries"]
linux = binaries["platforms"]["linux-x64"]
print(sys.argv[1], package["version"], binaries["baseUrl"] + "/" + linux["fileName"], linux["sha256"], sep="\t")' "$tag"
}

# upd_install_xpack <repo> <parent dir abs> <dir prefix> <stable link name>
# Installs the verified tarball into <parent>/<prefix>-<version> (side by side) and flips <parent>/<stable>.
upd_install_xpack() {
  local repo="$1" parent="$2" prefix="$3" stable="$4" info tag version url sha archive dest
  info="$(upd_xpack_info "$repo")" || return 1
  IFS=$'\t' read -r tag version url sha <<<"$info"
  dest="$parent/$prefix-$version"
  archive="$SDK_DIR/dl/$(basename "$url")"
  upd_fetch "$url" "$archive" "hex:$sha" || return 1
  upd_unpack "$archive" "$dest" 1 || return 1
  upd_patch "$dest"
  upd_flip "$parent/$stable" "$prefix-$version"
}

# ---------------------------------------------------------------- default rollback / prune

# Restore one recorded state line; returns non-zero when that step failed.
upd_undo_entry() {
  local kind="$1" path="$2" prev="$3" extra="$4" target="$SDK_DIR/$2"
  case "$kind" in
    link)
      if [[ -z "$prev" ]]; then
        echo "  $path had no previous target (left as is)"
        return 0
      fi
      ln -sfn "$prev" "$target.rb$$" && mv -T "$target.rb$$" "$target" && echo "  restored link $path -> $prev"
      ;;
    git)
      git -C "$target" checkout -q "$prev" && echo "  git $path -> ${prev:0:12}" || return 1
      if [[ -f "$target/.gitmodules" ]]; then
        git -C "$target" submodule update --init --recursive -q 2>/dev/null || echo "  (submodule re-sync incomplete for $path)"
      fi
      if [[ -n "$extra" ]]; then git -C "$target" checkout -q -B "$extra" "$prev" 2>/dev/null || true; fi
      ;;
    pip) uv pip install -q --python "$target" "$prev" && echo "  pip $prev restored" ;;
    west)
      git -C "$target/zephyr" checkout -q "$prev" \
        && (cd "$target" && west update --narrow -o=--depth=1) \
        && echo "  west workspace re-pinned to $prev"
      ;;
    repo)
      (cd "$target" && repo init -m "$prev" </dev/null && repo sync -j8 -c --no-clone-bundle) \
        && echo "  repo manifest $prev re-synced"
      ;;
    arduino) arduino-cli core install "$prev" >/dev/null && echo "  arduino core $prev restored" ;;
    file) cp -p "$prev" "$target" && echo "  file $path restored from $prev" ;;
    *)
      echo "  unknown state kind '$kind'" >&2
      return 1
      ;;
  esac
}

# Default sdk_rollback: replay $UPD_FILE (default <sdk>/.update-state), newest entry first.
sdk_rollback() {
  local file="${UPD_FILE:-$SDK_DIR/.update-state}" line kind path prev extra rc=0
  local -a lines
  if [[ ! -s "$file" ]]; then
    echo "$SDK: nothing recorded to roll back ($file)"
    return 0
  fi
  mapfile -t lines < <(tac "$file")
  for line in "${lines[@]}"; do
    IFS=$'\t' read -r kind path prev extra <<<"$line"
    upd_undo_entry "$kind" "$path" "$prev" "$extra" || rc=1
  done
  return "$rc"
}

# Versioned entries matching <glob> in <dir>, oldest first.
upd_family_members() {
  local dir="$1" glob="$2"
  (cd "$dir" && compgen -G "$glob" | sort -V) || true
}

# Prune one "stable-link|glob" family: entries older than the previous version go to the Trash.
# Prints one line per candidate and returns the number of candidates via UPD_PRUNED.
upd_prune_family() {
  local link="$1" glob="$2" current dir i current_index=-1 keep_from
  local -a members
  if [[ ! -L "$SDK_DIR/$link" ]]; then
    echo "  $link: not a symlink yet, skipped"
    return 0
  fi
  current="$(basename "$(readlink -f "$SDK_DIR/$link")")"
  dir="$(dirname "$SDK_DIR/$link")"
  mapfile -t members < <(upd_family_members "$dir" "$glob")
  for i in "${!members[@]}"; do
    [[ "${members[$i]}" == "$current" ]] && current_index="$i"
  done
  if ((current_index < 0)); then
    echo "  $link -> $current not in family $glob"
    return 0
  fi
  keep_from=$((current_index > 0 ? current_index - 1 : 0))
  for ((i = 0; i < keep_from; i++)); do
    UPD_PRUNED=$((UPD_PRUNED + 1))
    if upd_yes; then
      trash_move "$dir/${members[$i]}" "$SDK-${members[$i]}" && echo "  trashed ${members[$i]}"
    else
      echo "  would trash ${members[$i]}  (keeping ${members[$keep_from]} and $current)"
    fi
  done
}

# Default sdk_prune: plugins list "stable-link|glob" lines in sdk_prune_families; keeps current and previous.
sdk_prune() {
  local link glob
  if ! declare -F sdk_prune_families >/dev/null; then
    echo "$SDK: no versioned components to prune"
    return 0
  fi
  UPD_PRUNED=0
  while IFS='|' read -r link glob; do
    [[ -n "$link" ]] || continue
    upd_prune_family "$link" "$glob"
  done < <(sdk_prune_families)
  if ((UPD_PRUNED == 0)); then
    echo "$SDK: nothing older than the previous version"
  elif ! upd_yes; then
    echo "(dry run; 'mcu prune $SDK --yes' moves them to the Trash)"
  fi
}
