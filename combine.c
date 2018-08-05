precision highp float;
uniform sampler2D S;
uniform sampler2D R;
uniform vec2 X;

// Combines 2 buffers, blurred in directions [0,1] and [sin(120°), cos(120°)] to create hexagon
// bokeh pattern. Technique I got from DICE that they used in Frostbyte engine.
void main() {
	gl_FragColor = vec4(0);
	for (int i = 0; i < 10; i++) {
		gl_FragColor += texture2D(R, gl_FragCoord.xy / vec2(1280,720) + vec2(.866,-.5) * (float(i) + .5) * .003 * vec2(1,1.77));
		gl_FragColor += texture2D(R, gl_FragCoord.xy / vec2(1280,720) + vec2(-.866,-.5) * (float(i) + .5) * .003 * vec2(1,1.77));
		gl_FragColor += texture2D(S, gl_FragCoord.xy / vec2(1280,720) + vec2(.866,-.5) * (float(i) + .5) * .003 * vec2(1,1.77));
	}
	gl_FragColor /= 300.; // / 30 from above, / 10 from previous
}