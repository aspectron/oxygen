{
    'targets': [
        {
            'target_name': 'oxygen',
            'type': 'shared_library',
            'msvs_guid': '0C15F765-B4EA-4A86-B369-701BEE5828E0',
            'dependencies': [
                '<(jsx)/sdk/core/core.gyp:core',
                '<(jsx)/extern/extern.gyp:*',
                'oxygen_doc',
            ],
            'direct_dependent_settings': {
                'include_dirs': ['src'],
            },
            'defines': ['OXYGEN_EXPORTS'],
            'sources': [
                'src/gui.cpp',
                'src/gui.hpp',
                'src/keys.hpp',
                'src/oxygen.cpp',
                'src/oxygen.hpp',
                'src/video_modes.cpp',
                'src/video_modes.hpp',
            ],
            'conditions': [
                ['OS=="win"', {
                    'sources': [
                        'src/gui.windows.cpp',
                        'src/gui.windows.hpp',
                    ],
                }],
                ['OS=="mac"', {
                    'sources': [
                        'src/gui.mac.mm',
                        'src/gui.mac.hpp',
                    ],
                    'libraries': [
                        'AppKit.framework',
                        'CoreGraphics.framework',
                    ],
                }],
                ['OS not in ["win", "mac"]', {
                    'sources': [
                        'src/gui.x11.cpp',
                        'src/gui.x11.hpp',
                    ],
                    'libraries': ['-lX11', '-lXrandr', '-lGL'],
                }],
            ],
        },
        {
            'target_name': 'oxygen_doc',
            'type': 'none',
            'dependencies': [
                '<(jsx)/apps/jsx/jsx.gyp:*',
            ],

            'variables': {
                'jsx_app': '<(PRODUCT_DIR)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)',
                'doc_dir': 'doc/oxygen',
            },
            'conditions': [
                ['OS=="linux"', {
                    'variables': { 'jsx_app': '<(out_dir)/<(EXECUTABLE_PREFIX)jsx<(EXECUTABLE_SUFFIX)' },
                }],
            ],

            'actions': [
                {
                    'action_name': 'build_doc',
                    'inputs': ['rte/oxygen.js', 'src/oxygen.cpp'],
                    'outputs': ['<(doc_dir)/all.md'],
                    'action': ['<(jsx_app)', '<(jsx)/build/tools/gendoc/run.js',
                        '<(doc_dir)', 'rte/.+[.]js', 'src/.+[.]cpp',
                    ],
                    'msvs_cygwin_shell': 0,
                    'message': 'Building documentation...',
                },
            ],
        },
    ],
}
