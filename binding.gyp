{
    'variables': {
        'include_dirs': [
            'include',
            '<!(node -e require(\'v8pp\'))',
            '<!(node print_deps.js include)',
        ],
     },
    'targets': [
        {
            'target_name': 'oxygen',
            'dependencies': ['oxygen-doc'],
            'cflags_cc+': ['-std=c++11', '-fexceptions'],
            'msvs_settings': { 'VCCLCompilerTool': { 'ExceptionHandling': 1 } },
            'xcode_settings': { 'GCC_ENABLE_CPP_EXCEPTIONS': 'YES' },
            'include_dirs': ['<@(include_dirs)'],
            'direct_dependent_settings': {
                'include_dirs': ['<@(include_dirs)'],
            },
            'defines': ['OXYGEN_EXPORTS'],
            'defines!': ['V8_DEPRECATION_WARNINGS=1'],
            'sources': [
                'include/oxygen/gui.hpp',
                'include/oxygen/display.hpp',
                'include/oxygen/keys.hpp',
                'include/oxygen/oxygen.hpp',
                'include/oxygen/nodeutil.hpp',
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
                    'libraries': ['<!(node print_deps.js lib)'],
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
                    'action': ['<!(npm bin)/gendoc Oxygen', 'doc', '<@(_inputs)'],
                    'message': 'Building JavaScript API documentation...',
                },
            ],
        },
    ],
}
