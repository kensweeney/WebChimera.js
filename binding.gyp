{
    "targets": [{
            "target_name": "webchimera.js",
            "sources": [
                "deps/libvlc_wrapper/libvlc-sdk/include",
                "deps/libvlc_wrapper",
				"src"
            ],
            "include_dirs": [
                "src/webchimera.cpp"
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
