#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>

static int cmd_hello(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "hello %s from {{NAME}}", argc > 1 ? argv[1] : "world");
	return 0;
}
static int cmd_uptime(const struct shell *sh, size_t argc, char **argv)
{
	shell_print(sh, "uptime %lld ms", k_uptime_get());
	return 0;
}
SHELL_STATIC_SUBCMD_SET_CREATE(app_cmds,
	SHELL_CMD_ARG(hello, NULL, "hello [name]", cmd_hello, 1, 1),
	SHELL_CMD(uptime, NULL, "print uptime", cmd_uptime),
	SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(app, &app_cmds, "application commands", NULL);

int main(void) { return 0; }
