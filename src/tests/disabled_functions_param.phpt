--TEST--
Disable functions
--SKIPIF--
<?php if (!extension_loaded("snuffleupagus")) die "skip"; ?>
--INI--
sp.configuration_file={PWD}/config/config_disabled_functions_param.ini
--FILE--
<?php 
system("id");
system("echo win");
var_dump(array_sum([1,2,3,4,5]));
shell_exec("id");
echo shell_exec("echo 42");
strcmp("bla", "ble");
strncmp("bla", "ble", 2);
?>
--EXPECTF--
[snuffleupagus][0.0.0.0][disabled_function][drop] The call to the function 'system' in %a/disabled_functions_param.php:2 has been disabled, because its argument 'command' content (id) matched the rule '1'.
win
int(15)
[snuffleupagus][0.0.0.0][disabled_function][drop] The call to the function 'shell_exec' in %a/disabled_functions_param.php:5 has been disabled, because its argument 'cmd' content (id) matched the rule '3'.
42
[snuffleupagus][0.0.0.0][disabled_function][notice] The call to the function 'strcmp' in %a/tests/disabled_functions_param.php:7 has been disabled, because its argument 'str1' content (bla) matched the rule '5'.
[snuffleupagus][0.0.0.0][disabled_function][notice] The call to the function 'strncmp' in %a/tests/disabled_functions_param.php:8 has been disabled, because its argument 'str1' content (bla) matched a rule.
