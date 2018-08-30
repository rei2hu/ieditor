project(
	'ieditor',
	'cpp',
	version: '0.0.1',
	default_options: [
	  'c_std=c11',
	  'werror=true',
	  'warning_level=2'
	]
)

gtkdep = dependency('gtk+-3.0')

executable(
	'ieditor',
	files([
		'main.cpp',
	]),
	dependencies: [
		gtkdep
	]
)
