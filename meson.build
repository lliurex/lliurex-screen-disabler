project('lliurex-screen-disabler',['cpp'])

qt6 = import('qt6')
qt6_dep = dependency('qt6', modules: ['Gui','Widgets','Core'])

compiler = meson.get_compiler('cpp')
libdisplay = compiler.find_library('libdisplay-info')

executable('lliurex-screen-disabler', ['mainwindow.cpp','main.cpp'], dependencies: [qt6_dep,libdisplay], install: true)

executable('lliurex-screen-disabler-helper',['helper.cpp'], install: true,install_dir : 'libexec/')

install_data('net.lliurex.screen-disabler.policy', install_dir:'/usr/share/polkit-1/actions/')
