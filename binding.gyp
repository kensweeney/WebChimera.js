{
    "targets": [{
            "target_name": "webchimera.js",
            "sources": [
                "src/JsVlcAudio.cpp",
                "src/JsVlcDeinterlace.cpp",
                "src/JsVlcInput.cpp",
                "src/JsVlcPlayer.cpp",
                "src/JsVlcPlaylist.cpp",
                "src/JsVlcSubtitles.cpp",
                "src/JsVlcVideo.cpp",
                "src/NodeTools.cpp",
                "src/WebChimera.cpp"
            ],
            "include_dirs": [
                "<!(node -e \"require('node-addon-api')\")",
                "deps/libvlc_wrapper",
                "src"
            ],
            "dependencies": [
                "deps/libvlc_wrapper/binding.gyp:libvlc_wrapper"
            ],
            "cflags_cc!": ["-fno-exceptions"],
            "defines": ["NAPI_DISABLE_CPP_EXCEPTIONS"],
            "conditions": [
                ["OS=='linux' or OS=='mac'", {
                        "cflags_cc": [
                            "-std=c++20"
                        ]
                    }
                ],
                ["OS=='win'", {
                        "msvs_settings": {
                            "VCCLCompilerTool": {
                                "AdditionalOptions": [
                                    "/std:c++20",
                                    "/Zc:__cplusplus"
                                ]
                            }
                        }
                    }
                ]
            ]
        }
    ]
}
