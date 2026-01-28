// src/hs_perf/perf_shell.h (or similar)
#pragma once
#include <zephyr/shell/shell.h>

int cmd_hdect_perf_start(const struct shell *shell, size_t argc, char **argv);
int cmd_hdect_perf_stop (const struct shell *shell, size_t argc, char **argv);
int cmd_hdect_perf_reset(const struct shell *shell, size_t argc, char **argv);
int cmd_hdect_perf_show (const struct shell *shell, size_t argc, char **argv);
