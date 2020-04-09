{
    "targets": [
        {
            "target_name": "<(module_name)",
            "sources": [
                "src/PassThru/PassThruMisc.cpp",
                "src/PassThru/PassThruDevice.cpp",
                "src/ATE/ATE.cpp",
                "src/thread.cpp",
                "src/method.cpp",
                "src/main.cpp"
            ],
            'include_dirs': [
                "src/Common",
                "src/PassThru",
                "src/ATE",
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            'libraries': [],
            'dependencies': [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            'defines': [
                'NAPI_DISABLE_CPP_EXCEPTIONS'
            ]
        },
        {
            "target_name": "action_after_build",
            "type": "none",
            "dependencies": [ "<(module_name)" ],
            "copies": [{
                    "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
                    "destination": "<(module_path)"
            }]
        }
    ],
    "defines": [
        "NAPI_VERSION=<(napi_build_version)"
    ],
    "include_dirs": []
}