precision highp float;
uniform sampler2D S;
uniform sampler2D R;
uniform vec2 X;

vec4 textureDistorted(vec2 texcoord, vec2 direction) {
	return vec4(
		texture2D(R, texcoord + direction * -.015).r,
		texture2D(R, texcoord).g,
		texture2D(R, texcoord + direction * .015).b,
		1
	);
}

void main() {
    gl_FragColor = vec4(0);
	vec2 p = gl_FragCoord.xy / vec2(1280,720);
	vec2 pF = vec2(1) - p;
	vec2 ghostVec = (vec2(.5)-pF) * .25;
	vec2 ghostVecNorm = normalize(ghostVec);
	vec2 haloVec = ghostVecNorm * .5;
    vec4 streaks = vec4(0);
    vec4 tint = vec4(.8,.5,.2,1);
    vec4 focus = texture2D(S, p);

	// Ghosts
    for (int i = 0; i < 8; ++i) {
        vec2 offset = fract(pF + ghostVec * float(i));
        gl_FragColor += textureDistorted(
            offset,
            ghostVecNorm)
        * pow(1. - length(vec2(.5) - offset) / .707, 1.5) // Weight length(vec2(0.5) = ~.707
        * (tint = tint.zxyw);
    }

    // Halo big
    gl_FragColor += textureDistorted(fract(pF + haloVec), ghostVecNorm)
	    * pow(1. - length(vec2(.5) - fract(pF + haloVec)) / .7, 5.);

	// Halo small
	haloVec = ghostVecNorm * .3;
	gl_FragColor += textureDistorted(fract(pF + haloVec), ghostVecNorm)
	    * pow(1. - length(vec2(.5) - fract(pF + haloVec)) / .7, 10.) * .6;

    // Sharp glow + hyper space
    for (int i=-8; i<9; i++) {
        float dist = 1. - abs(float(i)) * .1;
        vec4 glow = (
            texture2D(S, p - vec2(1,0) * float(i) * .003) +
            texture2D(S, p - vec2(0,1.77) * float(i) * .003)
        ) * dist * dist * .02;
        focus += glow;
        streaks += texture2D(S,
                p - ghostVec * float(i) * .05 *
                    (smoothstep(110., 115., X.x) - smoothstep(118., 123., X.x)))
            / 17. + glow; // Keep in sync with engine shader
    }

    // Mix in unfocused space for early scenes
    gl_FragColor *= .05;
    gl_FragColor += focus.w < 1. ?
        mix(
            texture2D(R, p),
            streaks,
            smoothstep(12., 13., X.x) - smoothstep(75., 76., X.x) + smoothstep(90., 91., X.x)
        )
        : focus;

    gl_FragColor.w = 1.;
}