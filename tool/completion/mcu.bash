# shellcheck shell=bash
# bash completion for mcu: commands, SDK names, targets and examples (asked from `mcu __complete`).
# Enable for the current shell:   source "$BOX_MCU/completion/mcu.bash"   (BOX_MCU: box_env.sh)
# (add that line to ~/.bashrc yourself to keep it; mcu never edits rc files)

# Complete the current word of an mcu command line.
_mcu_complete() {
  local cur="${COMP_WORDS[COMP_CWORD]}" mcu="${COMP_WORDS[0]}" candidates
  local -a typed=("${COMP_WORDS[@]:1:COMP_CWORD-1}")
  if [[ "$cur" == -* ]]; then
    candidates="$("$mcu" __complete flags 2>/dev/null)"
  else
    candidates="$("$mcu" __complete next "${typed[@]}" 2>/dev/null)"
  fi
  mapfile -t COMPREPLY < <(compgen -W "$candidates" -- "$cur")
}

complete -F _mcu_complete mcu ./mcu
