// Player is added in during compilation. It was just easier to test the music while optimizing the
// player, by keeping it in separate file.

@PLAYER

// F is canvas created on HTML side
F.style.position = "fixed";
F.style.cursor = "none";
F.style.left = F.style.top = 0;
F.width = 1280;
F.height = 720;

// Rename webgl functions into shorter ones
for(i in G)
  G[i[0]+i[6]] = G[i];

G.ge("OES_texture_float"); // getExtension

var buffers = [];
var bufferTextures = [];
for (i=0; i<4; i++) {
    buffers.push(G.cF()); // createFramebuffer
    G.ba(36160, buffers[i]); // FRAMEBUFFER bindFramebuffer
    bufferTextures.push(G.cT()); // createTexture
    G.bx(3553, bufferTextures[i]); // bindTexture
    G.ta(3553, 10242, 33071); // texParameteri
    G.ta(3553, 10243, 33071);
    G.ta(3553, 10241, 9728);
    G.ta(3553, 10240, 9728);
    G.tg(3553, 0, 6408, 1280, 720, 0, 6408, 5126, null); // texImage2D
    G.fu(36160, 36064, 3553, bufferTextures[i], 0); // framebufferTexture2D
}

// Prepare all shaders we need. Again, placeholder replaced by compile.py.
var programs = [
  "@MAIN",
  "@BLUR",
  "@COMBINE",
  "@POST"
].map(x => {
    var s1 = G.cS(35632); // createShader
    var s2 = G.cS(35633);
    var program = G.cP(); // createProgram
    // All shaders use same input variables, even if they don't use them to save space
    G.sS(s1, "precision highp float;uniform sampler2D S,R;uniform vec2 X;" + x); // shaderSource
    G.sS(s2, "attribute vec4 p;void main(){gl_Position=p;}");
    G.ce(s1); // compileShader
    G.ce(s2);
    G.aS(program, s1); // attachShader
    G.aS(program, s2);
    G.lo(program); // linkProgram
    return program;
});

var outputBuffer = G.cB(); // createBuffer
G.bf(34962, outputBuffer); // bindBuffer
G.bD(34962, new Float32Array([1, 1, 1, -3, -3, 1]), 35044); // bufferData
G.vA(0, 2, 5126, 0, 0, 0); // vertexAttribPointer
G.eV(0); // enableVertexAttribArray

var renderFrame = () => {
    [
        // Rendering steps. We need to render scene and then do 4 post processing steps to get the
        // final image. This defines what buffers and parameters those steps takes in.
        [0,    mMixBuf.currentTime,  0,   0], // Render scene
        [1,    0,     1,   1,    0], // Y blur
        [1,    -.866, -.5, 2,    0], // XY blur
        [2,    0,     0 ,  3,    2, 1], // Combine
        [3,    mMixBuf.currentTime,  0,   null, 0, 3] // Post
    ].map(x => {
        // Use same rendering commands for all steps, even when they don't need them to save space

        G.ug(programs[x[0]]); // useProgram
        G.bf(34962, outputBuffer); // bindBuffer
        G.ba(36160, buffers[x[3]]); // bindFramebuffer

        G.aT(33984); // activeTexture
        G.bx(3553, bufferTextures[x[4]]); // bindTexture

        G.aT(33985);
        G.bx(3553, bufferTextures[x[5]]);

        G.uniform1i(G.gf(programs[x[0]], "S"), 0); // getUniformLocation
        G.uniform1i(G.gf(programs[x[0]], "R"), 1);
        G.uniform2fv(G.gf(programs[x[0]], "X"), [x[1], x[2]]);
        G.dr(4, 0, 3); // drawArrays
    });
    requestAnimationFrame(renderFrame);
};
renderFrame();
mMixBuf.play(); // This calls the Audio objects play, just reusing variable names.
