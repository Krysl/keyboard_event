echo CL=%CL%
set LC_ALL=.UTF-8
cmake --build  example\build\windows\plugins\keyboard_event

ls example\build\windows\runner\Debug\

rm example\build\windows\runner\Debug\keyboard_event_plugin.dll