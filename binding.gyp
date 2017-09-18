{
  "conditions": [
    ['OS=="win"', {
      "variables": {
        "has_avx2%": "<!(.\util\cpuinfo.exe 1 5 7)"
      }
    }]
  ],
  "targets": [
    {
      "target_name": "strdecode",
      "sources": [
        "src/strdecode.cc",
        "src/hex.cc"
      ],
      "include_dirs" : [
          "<!(node -e \"require('nan')\")"
      ],
      "cflags":[
        "-march=native"
      ],
      "conditions": [
        ['OS=="win" and has_avx2==1', {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "EnableEnhancedInstructionSet": 5 # /arch:AVX2
            }
          }
        }, {
          "msvs_settings": {
            "VCCLCompilerTool": {
              "EnableEnhancedInstructionSet": 3 # /arch:AVX
            }
          }
        }]
      ]
    }
  ]
}
