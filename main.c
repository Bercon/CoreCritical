precision highp float;
uniform sampler2D S;
uniform sampler2D R;
uniform vec2 X;

vec3 hitNormal;
vec3 PLANET_POSITION = vec3(
    800. - (X.x - 110.) * 7. // Normal movement
    - 600. * smoothstep(0., 125. - 110., X.x - 110.), // Planet movement during hyper space
    50.,
    0);
vec3 SPACE_POSITION = vec3(
    X.x * .01 // Spaces moves slowly by default
    + 7. * smoothstep(-25., 25., X.x) // Start of the intro, space moving fast
    + .5 * smoothstep(.0, 121. - 112., X.x - 112.), // Hyper space
    0,
    0);

vec3 SUN = vec3(100. - X.x * 1., -150, 0);

float SMOOTH_EXPLODE = smoothstep(.0, 184. - 165., X.x - 165.);
float PI = 3.14159;
float minDist;

int hitMaterial;

// ##################### FRACTALS #####################

// Fractal inspired by https://www.shadertoy.com/view/XlfGRj by Kali
// This creates texture for space, sun, planet and explosion
float fractal(vec3 p) {
    // Rotate to hide most visible artifacts
    p = vec3(p.y, mat2(cos(.4), -sin(.4), sin(.4), cos(.4)) * p.xz) + vec3(.5);
    p = abs(vec3(1)-mod(p,vec3(2))); // x, x*2
    float pa = .0;
    float a = .0;
    for (int i=0; i<13; i++) {
        p= abs(p) / dot(p,p) - .52;
        float len = dot(p,p) * .05;
        a += abs(len-pa);
        pa = len;
    }
    return a;
}

// ##################### PRIMITIVES #####################


// Box primitive distance function
float box(vec3 p, vec3 b) {
	vec3 v = abs(p) - b;
	return min(v.x>v.y?v.x>v.z?v.x:v.z:v.y>v.z?v.y:v.z,length(max(v,.0)));
}

// Stretched sphere primitive distance function
float ellipsoid(vec3 p, float x, float zy) {
    return (length(p / vec3(x,zy,zy)) - 1.) * min(x,zy);
}

// The structure beams distance function
float beams(vec3 p, float x, float y) { // length, diameter
    vec3 mp = p;
    mp.x = mod(p.x, y * .05);
    return
        max(
            max(
                max(
                    max(ellipsoid(p,  x * 5., y * .02), length(p) - x),
                    -max(ellipsoid(p,  x * 5.5, y * .015), length(p) - x * 1.1)
                    ),
                -box(mp, vec3(.045, .012, 1) * y)),
            -box(mp, vec3(.045, 1, .012) * y)
        );
}

// ##################### FIELD #####################

// Scenes combined distance function, includes ship, planet and sun
float field(vec3 p) {
    if (dot(p,p) > 9.) { // If not around spaceship, check for planet and sun
        vec3 planetP = p - PLANET_POSITION;
         // Planet is a shell, radius 40., thickness 1.8
        float planet = abs(length(planetP) - 40. - sin(SMOOTH_EXPLODE * 1.6) * 85.) - 1.8;
        // Core is just a sphere
        float core = length(planetP) - 35. - sin(SMOOTH_EXPLODE * 1.6) * 175.;
        if (planet < 1.) { // Performance optimization
            float kali = fractal(planetP * .01);
            kali /= kali + 2.;
            planet += kali * smoothstep(.0, 175. - 135., X.x - 135.) * 5.;
        }
        if (core < 1.) // Performance optimization
            core += fractal(planetP * .01 + X.x * vec3(.02))
                * smoothstep(.0, 175. - 135., X.x - 135.) * 5.;
        float sun = length(p - SUN) - 60.;
        float res = sun;
        hitMaterial = 1; // Sun
        if (core < res) {
            res = core;
            hitMaterial = 2; // Lava/Core
        }
        if (planet < res) {
            res = planet;
            hitMaterial = 3; // Planet
        }
        return res;
    }

    // Rotate spaceship and the torus
    float a =  -X.x * .133;
    p = vec3(p.x + .5, mat2(cos(a), -sin(a), sin(a), cos(a)) * p.yz);
    a *= -4.;
    vec3 torusP = vec3(p.x, mat2(cos(a), -sin(a), sin(a), cos(a)) * p.yz) - vec3(1.4, 0, 0);
    vec3 mp;

    // Compute bounding boxes to increase performance
    float boxSolar = box(p-vec3(.2,0,0), vec3(.6, .03, 1.3)); // Solar panels
    float boxCore = box(p, vec3(1.6, .3, .3)); // Center structure
    float boxTorus = box(p-vec3(1.4,0,0), vec3(.2, .9, .9)); // Torus
    float res = min(min(boxSolar, boxCore), boxTorus);

    if (res > .21) // Performance optimization
        return res;

    res = 10.;
    if (boxTorus < .4) { // Performance optimization
        // Torus
        a = abs(PI - mod(atan(torusP.z, torusP.y) * 32., PI * 2.));
        a = min(a, PI - abs(PI - mod(torusP.x * 64., PI * 2.)));
        a = .15 - smoothstep(.0,.15, a);
        res =
            max(
                max(
                    max(
                        length(vec2(length(torusP.yz) - .8, torusP.x)) - .15,
                        length(vec2(length(torusP.yz) - .45, torusP.x)) - .4
                        ),
                    length(vec2(length(torusP.yz) - 1.15, torusP.x)) - .4
                ),
                length(abs(torusP) - vec3(0, .6, .6)) - .5
            );

        res += a * .002;
    }

    // Basic body modules
    if (boxCore < .4) {
        a = abs(PI - mod(atan(p.z, p.y) * 8., PI * 2.));
        a = min(a, PI - abs(PI - mod(p.x * 64., PI * 2.)));
        a = .25 - smoothstep(.0,.25, a);
        mp = abs(p - vec3(.2, 0, 0)) - vec3(.2, 0, 0);
        res = min(res, max(ellipsoid(mp, .5, .1), length(mp) - .2) + a * .002);
        mp = abs(p-vec3(1, 0, 0)) - vec3(.2, 0, 0);
        res = min(res, max(ellipsoid(mp, .5, .1), length(mp) - .2) + a * .002);
    }

    hitMaterial = 5; // White
    float previousMatDistance = res;

    // Engines
    if (p.x < .0 && boxCore < .4) {
        mp = abs(p-vec3(-1, 0, 0)) - vec3(0, .1, 0);
        res = min(res, max(ellipsoid(mp, 3., .08), length(mp) - .2));
    }

    if (res < previousMatDistance)
        hitMaterial = p.x < -1.195 ? 4 : 6; // Metal & Engine
    previousMatDistance = res;

    // Beams
    if (boxCore < .2) // Performance optimization
        res = min(res, beams(p, 1.55, 2.2)); // Central
    if (boxSolar < .1) // Performance optimization
        res = min(res, beams(p.zyx - vec3(0, 0, .2), 1.3, 1.5)); // Cross for solar
    if (boxTorus < .11) { // Performance optimization
        res = min(res, beams(torusP.zyx, .83, 2.)); // Rotating
        res = min(res, beams(abs(torusP.yxz) - vec3(0, 0, .8), .15, 2.)); // Torus structure
    }

    // Tanks
    if (p.x < .0 && boxCore < .4) { // Performance optimization
        mp = p.yzx - vec3(.17, 0, -.3);
        res = min(res, max(ellipsoid(mp, 2., .07), length(mp) - .13));
        mp = p.zyx - vec3( .17, 0, -.3);
        res = min(res, max(ellipsoid(mp, 2., .07), length(mp) - .13));
        mp = p.zyx - vec3( .17, 0, -.45);
        res = min(res, max(ellipsoid(mp, 2., .07), length(mp) - .13));
        mp = p.zyx - vec3( .17, 0, -.6);
        res = min(res, max(ellipsoid(mp, 2., .07), length(mp) - .13));
    }

    if (res < previousMatDistance)
        hitMaterial = 6; // Metal
    previousMatDistance = res;

    // Solar panels
    if (boxSolar < .5) {
        mp = abs(p - vec3(.2, 0, 0));
        res = min(res, box(mp - vec3(.3, .02, 1.1), vec3(.276, .003, .126)));
        res = min(res, box(mp - vec3(.3, .02, .8), vec3(.276, .003, .126)));
        res = min(res, box(mp - vec3(.3, .02, .5), vec3(.276, .003, .126)));
    }

    if (res < previousMatDistance)
        hitMaterial = 7; // Solar

    return res;
}

// ##################### RAYMARCH #####################

// One raymarching function for camera, reflections and shadows. This is done for size reasons and
// spliting this into several didn't provide that much added performance. Perhaps I just did
// something wrong.
float raymarch(vec3 origin, vec3 dir, float dist, float maxDist) {
	float minStep = .00005;
    minDist = 1000.;
	for (int i = 0; i < 100; i++) {
		vec3 p = origin + dir * dist;
		float f = field(p);
        minDist = min(minDist, 10. * f / dist); // Use to create soft shadows
		if (f <  .0003) {
			hitNormal = normalize(vec3(
				f - field(p - vec3(.0001, 0, 0)),
				f - field(p - vec3(0, .0001, 0)),
				f - field(p - vec3(0, 0, .0001))));
			return dist;
		}
		dist += min(minStep, f);
        minStep += .0003 * float(i*i);
		if (dist > maxDist) {
		    hitMaterial = 0;
	        return maxDist;
        }
	}
    hitMaterial = 0;
	return maxDist;
}

// ##################### Shading #####################

// Shading is used twice, once for camera ray and once for reflection. It's really heavy function,
// so creating a simplified version for reflections would have been smarter if I'd have managed to
// squeeze it into 4096 bytes.
vec3 shade(vec3 hit, vec3 dir, vec3 normal, int mat) {
    if (mat == 0) { // Space
        // Fractal inspired by https://www.shadertoy.com/view/XlfGRj by Kali
    	float s=.0;
    	vec3 v=vec3(0);
    	for (int r=0; r<15; r++) {
    		float a = min( // Sample twice to reduce noise
        		fractal(SPACE_POSITION + s * dir * 2.),
        		fractal(SPACE_POSITION + s * dir * 2. + vec3(.003)));
    		v+=vec3(s, s*s, s*s*s*s)*a*a*a*.000003;
    		s+=.1;
    	}
    	v /= length(v) + 3.;
    	return v * 20.;
    }
    if (mat == 1) {// Sun
        // 9. position offset gives better texture at right time
        float sun = fractal(hit * .012 + vec3(X.x * .01 + 9.)) * .5;
        sun /= sun + 1.;
        return vec3(7.5,1,.25) * (.1 + sun);
    }
    if (mat == 2) // Lava
        return vec3(2, .5, .3)
        * (.01 + fractal(hit * .01))
        * (.15 + SMOOTH_EXPLODE * 5.);

    if (mat == 4) // Engine
        return vec3(5,5,25) * (smoothstep(110., 110.5, X.x) - smoothstep(118.5, 119., X.x));

    // BASIC SHADING
    float surface = clamp(dot(normal, normalize(SUN)), .0, 1.);
    float sunLight = .0;
    float ao = .0;

    if (mat == 3) { // Planet
        vec3 v = hit - PLANET_POSITION;
        float ground = fractal(v * .03);
        ground /= ground + 1.;
        float add = ground;
        // Distort second fractal by first to create more interesting texture
        ground = fractal(v * (4. + ground) * .01);
        ground /= ground + 1.;
        // Create stripes/waves by evaluating fractal in one dimension only
        float waves = fractal(vec3(v.z * .05 + v.y * .02));
        waves /= waves + 1.;
        return mix(
               vec3(1, .6, .25),
               vec3(.05, .04, .04),
               ground * .7 + add * .3
            )
            * (waves * .6 + .4)
            * (
                surface // Surface normal
                    * clamp(dot(normalize(v), normalize(SUN)), .0, 1.) // Planet normal
                    * vec3(1.2,1.1,1) * .9 // Sun color
                + vec3(.1,.1,.13) // Fill light
            );
    }

    // SHADOWS
    if (raymarch(hit + normal * .002,  normalize(SUN - hit), .01, 2.) == 2.)
        sunLight = clamp(.0, 1., minDist); // raymarch() sets minDist global

    // AMBIENT OCCLUSION
	for (int i = 0; i < 5; i++)
	    ao += field(hit + normal * (.05 + float(i) * .05));

    vec3 lighting = ao * 1.34 * (sunLight * surface * vec3(1.2,1.1,1) + vec3(.1,.1,.13));

    if (mat == 5) // White
        return lighting;

    if (mat == 6) // Metal
        return lighting * .3;

    // Solar panel
    float a = -X.x * .133; // Must be same as in field
    hit = vec3(hit.x, mat2(cos(a), -sin(a), sin(a), cos(a)) * hit.yz);
    return lighting * mix(vec3(.4), vec3(.14, .12, .5),
        smoothstep(.0,.1,
            min(
                abs(1. - mod(hit.x * 40., 2.)),
                abs(1. - mod(hit.z * 40., 2.))
            )
       )
    );
}

// ##################### Main #####################

void main() {
    float orbitSmooth = smoothstep(.0, 109. - 45., X.x - 45.);
    float camDistance =
        10.
        - smoothstep(.0, 55. - 45., X.x - 45.) * 6.5
        - sin(PI * orbitSmooth) * .5;
    float angle = PI * (
        .12
        + smoothstep(-25., 25., X.x) * 2. // Panning around with just space
        + orbitSmooth * 2.4 // Orbit in the beginning
        + smoothstep(.0, 155. - 119., X.x - 119.) * .84 // Flyby, keeping planet in focus
    );

    // CAMERA

	vec3 camOrigin = vec3(
	    camDistance * sin(-angle) + 100. - smoothstep(.0, 48., X.x * .5 + 48. * .5) * 100.,
	    camDistance * cos(-angle),
	    sin(PI * 1.96 * orbitSmooth) * 4.
	);

    vec2 pos = (vec2(1280,720)*.5-gl_FragCoord.xy)/720.; // Screen space: -.5 .. .5
    vec3 vx = normalize(
        vec3( // Cam Target
                100. - smoothstep(0., 48., X.x * .5 + 48. * .5) * 100.,
                0,
                smoothstep(0., 35. - 25., X.x - 25.) * 10. - 10. // Panning down to reveal sun
        ) - camOrigin);
    vec3 vy = normalize(vec3(-vx.y, vx.x, 0));
    vec3 camDir = normalize(vx + pos.x * vy * .6 + pos.y * cross(vx, vy) * .6);

    // RENDER

    float hitDist = raymarch(camOrigin, camDir, 1., 1000.);
    camOrigin = camOrigin + camDir * hitDist; // Reuse camOrigin as hit point
    int mat = hitMaterial;
    vec3 normal = hitNormal;
    vec3 result = shade(camOrigin, camDir, normal, mat);

    if (mat > 4) { // Materials 5-7 reflect
        float reflectionAngle = (1. + dot(camDir, normal)) * .5;
        reflectionAngle *= reflectionAngle;
        camDir -= normal * dot(camDir, normal) * 2.;
        hitDist = raymarch(camOrigin, camDir, .03, 300.);
        result = mix(
            result,
            shade(camOrigin + camDir * hitDist, camDir, hitNormal, hitMaterial),
            reflectionAngle + (mat < 6 ? .0 : .4) // Metal and solar reflects more
        );
    }

    // Turn screen fully white at 179 seconds, the end
    gl_FragColor = vec4(X.x>179.?vec3(1):result, float(mat));
}
