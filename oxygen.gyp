{
    'targets': [
        {
            'target_name': 'oxygen',
            'type': 'shared_library',
            'msvs_guid': '0C15F765-B4EA-4A86-B369-701BEE5828E0',
            'dependencies': ['<(jsx)/sdk/core/core.gyp:core'],
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
                ['OS=="linux"', {
                    'sources': [
                        'src/gui.linux.cpp',
                        'src/gui.linux.hpp',
                    ],
                    'libraries': ['-lX11', '-lXrandr', '-lGL'],
                }],
            ],
        },
    ],
}
