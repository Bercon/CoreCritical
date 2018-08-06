Core Critical by HBC
==========
Source codes for 4k intro HBC-00016: Core Critical by ½-bit Cheese. It placed 1st in Assembly 2018 4k intro competition.

Pouet link: https://www.pouet.net/prod.php?which=77372

Files
-------------------------
    blur.c           Shader for calculating blur in one dimension
    combine.c        Shader for combining two one dimensionally blurred buffers
    post.c           Shader for calculating lens flares and other post effects
    main.c           Shader for rendering the scene

    player_min.js    Music player and song data
    webgl_min.js     Rendering pipeline

    extern.js        Definitions for Closure Compiler
    compile.py       Python script to compile sourcce files into final entry


Tools
-------------------------
You need following tools to compile the final entry from source files. 
  
* Shader Minifier 1.1.6 (https://github.com/laurentlb/Shader_Minifier)
    * For minimizing shaders
* Google Closure Compiler v20180610 (https://developers.google.com/closure/compiler/)
    * For minimizing javascript
* JsExe 1.1.1 (http://www.pouet.net/prod.php?which=59298)
    * For compressing final entry into PNG
* Python 3
    * For running the compile.py script
* Java
    * For running Closure Compiler
    
For copyright reasons none of the tools are included here and you must download them yourself. Place shader_minifier.exe, jsexe.exe and closure-compiler-v20180610.jar into the root folder of the project or modify their paths in compile.py. Then run compile.py to produce the final <=4096 byte entry file.


Crew
-------------------------
* Bercon - Code, Graphics
* Valtteri - Music
