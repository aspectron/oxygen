{
    'includes': ['common.gypi'],
    'variables': {
        'include_dirs': [
            'include',
            '<!(node -e require(\'v8pp\'))',
            '<!@(pkgdeps include nitrogen)',
        ],
     },
    'targets': [
        {
            'target_name': 'oxygen',
            'dependencies': ['oxygen-doc'],
            'msvs_settings': { 'VCLinkerTool': {
                'DelayLoadDLLs': ['nitrogen.node']
            }},
            'include_dirs': ['<@(include_dirs)'],
            'direct_dependent_settings': {
                'include_dirs': ['<@(include_dirs)'],
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
                    'libraries': ['<!@(pkgdeps lib nitrogen)'],
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
            'actions': [
                {
                    'action_name': 'gendoc',
                    'inputs': ['src/oxygen.cpp'],
                    'outputs': ['doc/all.md'],
                    'action': ['gendoc Oxygen', 'doc', '<@(_inputs)'],
                    'message': 'Building JavaScript API documentation...',
                },
            ],
        },
    ],
}
