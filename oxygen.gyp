{
    'targets': [
        {
            'target_name': 'oxygen',
            'type': 'shared_library',
            'msvs_guid': '0C15F765-B4EA-4A86-B369-701BEE5828E0',
            'dependencies': [
                '<(jsx)/jsx-lib.gyp:jsx-lib',
                '<(jsx)/extern/extern.gyp:*',
                'oxygen-doc',
            ],
            'include_dirs': ['include'],
            'direct_dependent_settings': {
                'include_dirs': ['include'],
            },
            'defines': ['OXYGEN_EXPORTS'],
            'sources': [
                'include/oxygen/gui.hpp',
                'include/oxygen/display.hpp',
                'include/oxygen/keys.hpp',
                'include/oxygen/oxygen.hpp',
                'src/gui.cpp',
                'src/oxygen.cpp',
            ],
            'conditions': [
                ['OS=="win"', {
                    'sources': [
                        'src/display.windows.cpp',
                        'src/gui.windows.cpp',
                        'include/oxygen/gui.windows.hpp',
                    ],
                }],
                ['OS=="mac"', {
                    'sources': [
                        'src/display.mac.mm',
                        'src/gui.mac.mm',
                        'include/oxygen/gui.mac.hpp',
                    ],
                    'libraries': [
                        'AppKit.framework',
                        'CoreGraphics.framework',
                        'IOKit.framework',
                    ],
                }],
                ['OS not in ["win", "mac"]', {
                    'sources': [
                        'src/display.x11.cpp',
                        'src/gui.x11.cpp',
                        'include/oxygen/gui.x11.hpp',
                    ],
                    'libraries': ['-lX11', '-lXrandr', '-lGL'],
                }],
            ],
        },
        {
            'target_name': 'oxygen-doc',
            'type': 'none',
            'dependencies': ['<(jsx)/jsx-app.gyp:jsx-app'],
            'variables': {
                'jsx_app': '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)',
                'doc_dir': 'doc/oxygen',
                'conditions': [
                    ['OS=="linux"', {
                        'jsx_app': '<(out_dir)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)'
                    }],
                ],
            },
            'actions': [
                {
                    'action_name': 'gendoc',
                    'inputs': ['rte/oxygen.js', 'src/oxygen.cpp'],
                    'outputs': ['<(doc_dir)/all.md'],
                    'action': ['<(jsx_app)', '<(jsx)/build/tools/gendoc/run.js', '<(doc_dir)', '<@(_inputs)'],
                    'msvs_cygwin_shell': 0,
                    'message': 'Building JavaScript API documentation...',
                },
            ],
        },
    ],
}
