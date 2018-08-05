import os
import subprocess

SHADER_MINIFIER = 'shader_minifier.exe'
CLOSURE_COMPILER = 'closure-compiler-v20180610.jar'
JS_EXE = 'jsexe.exe'

TMP = 'temp'

def compileShader(name):
    outputFileName = TMP + '/stripped_' + name
    outputFileName2 = TMP + '/minified_' + name
    with open(name) as input, open(outputFileName, 'w') as output:
        for line in input:
            if (line.startswith('precision highp float;')):
                pass # We want to strip this so shader minifier can read the file
            else:
                output.writelines(line)
    subprocess.call([
        SHADER_MINIFIER,
        '--preserve-externals',
        '--format', 'none',
        '--field-names', 'rgba',
        #'--smoothstep', # Doesn't compile properly
        '-v',
        '-o', outputFileName2,
        outputFileName
    ])
    with open(outputFileName2) as input:
        return input.readline().replace('uniform sampler2D S,R;uniform vec2 X;', '')

if (not os.path.isdir(TMP)):
    os.mkdir(TMP);
filelist = [f for f in os.listdir(TMP) if f.endswith(".c") or f.endswith(".js") or f.endswith(".html")]
for f in filelist:
    os.remove(os.path.join(TMP, f))

main = compileShader('main.c')
blur = compileShader('blur.c')
post = compileShader('post.c')
combine = compileShader('combine.c')

playerCode = '';
with open('player_min.js') as input:
    playerCode = input.read();

injectedName = TMP + '/injected_main.js'
with open('webgl_min.js') as input, open(injectedName, 'w') as output:
    for line in input:
        line = line.replace('@MAIN', main)
        line = line.replace('@BLUR', blur)
        line = line.replace('@POST', post)
        line = line.replace('@COMBINE', combine)
        line = line.replace('@PLAYER', playerCode)
        line = line.replace('1280', '1920')
        line = line.replace('720', '1080')
        output.writelines(line)

jsOutputName = TMP + '/minified_main.js'
subprocess.call('javaw -jar ' + CLOSURE_COMPILER
                + ' --compilation_level ADVANCED_OPTIMIZATIONS'
                #+ ' --formatting PRETTY_PRINT'
                + ' --js ' + injectedName
                + ' --externs extern.js '
                + ' --js_output_file ' + jsOutputName
                , shell=True)

withContextName = TMP + '/contextified_main.js'
with open(jsOutputName) as input, open(withContextName, 'w') as output:
    for index, line in enumerate(input):
        if (index == 0):
            line = 'var G,' + line[4:]
        line = line.replace('in G)', 'in G=F.getContext("webgl"))')
        line = line.replace('function(a){', 'a=>{')
        line = line.replace('function(a,m){', '(a,m)=>{')
        line = line.replace('function y(){', 'var y=a=>{')
        line = line.replace('requestAnimationFrame(y)}', 'requestAnimationFrame(y)};')
        line = line.replace('G.ge("OES_texture_float")', 'with(G){G.ge("OES_texture_float")')
        line = line.replace('G.', '')
        line = line.replace('1E-5', '.00001')
        line = line.replace('5e-05', '.00005')
        line = line.replace('2e-05', '.00002')
        line = line.replace('3e-06', '.000003')
        line = line.replace('\n', '')
        output.writelines(line)
    output.writelines('}')

pngifiedName = TMP + '/pngified.html'
subprocess.call([
    JS_EXE,
    #'--fromcharcode=S',  # -- not an error
    '-cn',
    '-mn',
    '-ps',
    '-bb',
    withContextName,
    pngifiedName
])

finalEntryName = TMP + '/index.html'
with open(pngifiedName, 'rb') as input, open(finalEntryName, 'wb') as output:
    data = input.read()
    data = data.replace(
        '<canvas id=V>'.encode('utf-8'),
        '<canvas id=F><canvas id=V>'.encode('utf-8')
    )
    output.write(data)

bytes = os.path.getsize(finalEntryName)

statsName = TMP + '/stats.txt'
with open(statsName, 'a') as output:
    output.writelines('{0} ({1})\n'.format(bytes, bytes - 4096))

print('Entry filesize {0} bytes'.format(bytes))
print('From budjet {0} bytes'.format(bytes - 4096))
