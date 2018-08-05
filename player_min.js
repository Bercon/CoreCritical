/*
* The player and music is done using https://github.com/mbitsnbites/soundbox . Obviously it's been
* heavily minimizied and all non-essential functionality has been striped away, such as performance
* since this is only run in the beginning of the demo to generate the music. This doesn't run when
* audio is actually playing.
*
* Original license for the minified player, that was used as a base for the version seen here
*
* Copyright (c) 2011-2013 Marcus Geelnard
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software
*    in a product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
* 2. Altered source versions must be plainly marked as such, and must not be
*    misrepresented as being the original software.
*
* 3. This notice may not be removed or altered from any source
*    distribution.
*
*/


var mMixBuf = new Int32Array(15812480); // Create work buffer (initially cleared)
var wave = new Uint8Array(31625004 /*44 + 15812480 * 2*/);
var i;
var j;

// Header, precalculated values for this particular song
wave.set([82, 73, 70, 70, 36, 143, 226, 1, 87, 65, 86, 69, 102, 109, 116, 32, 16, 0, 0, 0, 1, 0,
  2, 0, 68, 172, 0, 0, 16, 177, 2, 0, 4, 0, 16, 0, 100, 97, 116, 97, 0, 143, 226, 1]);

[ // Instruments, this determines the sound of each instrument
    [192,143,0,7744,62500,3,26,1,0,32,63760],
    [192,192,47524,31684,73984,2,2,1,21,121,63760],
    [194,170,16,0,14400,1,255,0.18,20,71,15940],
    [146,224,234256,14884,61504,2,15,1,31,28,47820],
    [255,228,256,0,5184,3,15,0.765,30,16,15940]
].map((X, I) => {
    var p, row, i2, k, rsample, f, lsample;
    var chnBuf = new Int32Array(15812480);
    var low = 0, band = 0, high;
    for (p = 0; p <= 30; p++) {
      var colls = [ // Columns, these are the notes that are played
        [[25,,,,,,,,,,,,,,,,25,,,,,,,,,,,,,,,,37,,,,,,,,,,,,,,,,37],
          [30,,,,,,,,37,,,,,,,,37,,,,,,,,32,,,,,,,,42,,,,,,,,49,,,,,,,,49,,,,,,,,44],
          [33,,,,,,,,30,,,,,,,,35,,,,,,,,32,,,,,,,,45,,,,,,,,42,,,,,,,,47,,,,,,,,44]],
        [[57,,,,,,,,,,,,,,,,57,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,69],
          [57,,,,,,,,,,,,,,,,57,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,68,,,,,,,,,,,,,,,,68],
          [66,,,,,,,,,,,,,,,,66,,,,,,,,,,,,,,,,57,,,,,,,,,,,,,,,,57,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,69],
          [54,,,,,,,,,,,,,,,,54,,,,,,,,,,,,,,,,62,,,,,,,,66,,,,,,,,62,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,74,,,,,,,,,,,,,,,,74],
          [55,,,,,,,,,,,,,,,,55,,,,,,,,,,,,,,,,62,,,,50,,,,38,,,,,,,,62,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,75,,,,,,,,,,,,,,,72,75],
          [55,39,,,,,,,,,,,,,,,55,39,,,,,,,,,,,,,,,63,,,,,,,,,,,,,,,,63,,,,,,,,,,,,,,,,70,,,,,,,,,,,,,,,,70,,,,,,,,,,,,,,,,75,,,,,,,,,,,,,,,,75],
          [54,38,,,,,,,,,,,,,,,54,50,,,,,,,,,,,,,,,62,,,,,,,,,,,,,,,66,62,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,69,,,,,,,,,,,,,,,,74,,,,,,,,,,,,,,,,74],
          [49,,,,,,,,,,,,49],
          [30,,,,,,,,,,,,,,,,37],
          [38,,,,,,,,,,,,,,,,40],
          [18,,,,,,,,,,18,,,,,,,,,,18,,,,,,,,,,,,7,,,,,,,,,,7,,,,,,,,,,7]],
        [[78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,77,77,77,77,77,77,77,77,77,77,77,77,77,77,77,77],
          [78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80],
          [85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,77,77,77,77,77,77,77,77,77,77,77,77,77,77,77,77],
          [78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,78,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,80,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85,85]],
        [[66]],
        [[66,68,69,73,66,68,69,73,66,68,69,73,66,68,69,73,65,68,69,73,65,68,69,73,65,68,69,73,65,68,69,73],
          [62,66,69,73,62,66,69,73,62,66,69,73,62,66,69,73,64,68,69,73,64,68,69,73,65,68,69,73,65,68,69,73]]
      ][I][
            [ // Patterns, which column of notes to play from above and when
              [,,,,,,,,,,0,0,0,1,2,1,2,1,2,1,2,1,2,1,2,1,2,0,0,0],
              [0,0,1,2,1,3,4,5,6,7,7,7,7,,,8,9,8,9,8,9,8,9,8,9,8,9,7,7,10,10],
              [,,,,,,,,,,,,,,,,,,,0,1,2,3,2,3,2,3],
              [,,,,,,,,,,,,,,,,,,,0,,,,0,0,0,0,0,0,0],
              [,,,,,,,,,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1]
            ][I][p]
        ];
      for (row = 0; row < 32; row++) {
        if (colls)
          for (i = 0; i < 4; i++) {
              if (colls[row + i * 32] > 0) {
                  var noteBuf = new Int32Array(X[2] + X[3] + X[4]);
                  var c1 = 0, c2 = 0;
                  var e;
                  // Generate one note (attack + sustain + release)
                  for (j = 0; j < X[2] + X[3] + X[4]; j++) {
                      // Envelope
                      e = 1;
                      if (j < X[2])
                          e = j / X[2];
                      else if (j >= X[2] + X[3])
                          e -= (j - X[2] - X[3]) / X[4];
                      // Calculate note frequencies for the oscillators
                      c1 += 0.00396 * Math.pow(2, (colls[row + i * 32] - 47) / 12); // 0.00395950375
                      c2 += 0.00396 * Math.pow(2, (colls[row + i * 32] - 35) / 12); // 0.00395950375
                      // Add to (mono) channel buffer
                      noteBuf[j] = (
                          Math.sin(c1 * 6.283184) * X[0] // TODO: PI * 2
                          + Math.sin(c2 * 6.283184) * X[1]
                      ) * e * 80;
                  }
                  for (j = 0, i2 = (p * 32 + row) * 7970 * 2; j < X[2] + X[3] + X[4]; j++, i2 += 2)
                      chnBuf[i2] += noteBuf[j];
              }
          }
        for (j = 0; j < 7970; j++) {
          k = ((p * 32 + row) * 7970 + j) * 2;
          f = 1.5 * Math.sin(X[6] * .00308); // 43.23529 * 3.141592 / 44100); // FX_FREQ
          low += f * band;
          high = X[7] * (chnBuf[k] - band) - low; // FX_RESONANCE
          band += f * high;
          rsample = X[5] == 3 ? band : X[5] == 1 ? high : low;
          // Distortion
          if (X[8] * .00001) { // FX_DIST
            rsample *= X[8] * .00001;
            rsample = rsample < 1 ? rsample > -1 ? Math.sin(rsample * .25 * 6.283184) : -1 : 1;
            rsample /= X[8] * .00001;
          }
          lsample = rsample *= X[9] / 64; //  FX_DRIVE
          if (k >= X[10]) { // FX_DELAY_TIME
            lsample += chnBuf[k-X[10]+1] * 93 / 255; //  FX_DELAY_TIME, FX_DELAY_AMT
            rsample += chnBuf[k-X[10]] * 93 / 255; //  FX_DELAY_TIME, FX_DELAY_AMT
          }
          mMixBuf[k] += chnBuf[k] = lsample;
          mMixBuf[k + 1] += chnBuf[k + 1] = rsample;
        }
      }
    }
});

// Append actual wave data
for (i = 0, j = 44; i < 15812480; i++) {
    wave[j++] = mMixBuf[i] < -32767 ? -32767 : (mMixBuf[i] > 32767 ? 32767 : mMixBuf[i])
      & 255;
    wave[j++] = (mMixBuf[i] < -32767 ? -32767 : (mMixBuf[i] > 32767 ? 32767 : mMixBuf[i]) >> 8)
      & 255;
}

mMixBuf = new Audio();
mMixBuf.src = URL.createObjectURL(new Blob([wave], {type: "audio/wav"}));
