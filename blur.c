precision highp float;
uniform sampler2D S;
uniform sampler2D R;
uniform vec2 X;

// Blur's image in one direction given by X
void main() {
	gl_FragColor = vec4(0);
	for (int i = 0; i < 10; i++)
		gl_FragColor += texture2D(S,
		    gl_FragCoord.xy / vec2(1280,720) + X * (float(i) + .5) * .003 * vec2(1,1.77));
}