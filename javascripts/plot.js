'use strict';

var pInfo = {};

function procData(d) {
  var canvas0 = document.getElementById("myCanvas"),
      ctx = canvas0.getContext("2d"),
      pop = document.getElementById("myOverlay").getContext("2d");

  // d.t = d.t.slice(0,d.t.length-30);
  // d.y = d.y.slice(0,d.t.length);

  if ('error' in d) { // Server sent over an error
    alert(d.error);
    throw(d.error);
  }

  document.title = d.title;

  if (d.t.length !== d.y.length) {
    myAlert("Lengths of t(" + d.t.length + ") and y(" + d.y.length + 
            ") not the same.");
  }

  if (d.t.length < 2) {
    myAlert("Insufficient data, " + d.t.length);
  }

  initPopInfo(d, canvas0, pop);

  // Display start/end dates in the form
  updateForm(d);

  drawData(d, ctx);

  canvas0.addEventListener("mousemove", function() {myMouse(event);});
  canvas0.addEventListener("touchmove", function() {myMouse(event);});
  window.onresize = function(e) {drawData(d, ctx);};
} // procData

function drawData(d, ctx) {
  var dpr = window.devicePixelRatio || 1,
      bsr = ctx.webkitBackingStorePixelRatio ||
            ctx.mozBackingStorePixelRatio ||
            ctx.msBackingStorePixelRatio ||
            ctx.oBackingStorePixelRatio ||
            ctx.backingStorePixelRatio || 1,
      ppi = 96 * dpr / bsr,
      nWidth = Math.ceil((window.innerWidth || 
                          document.documentElement.clientWidth || 
                          document.body.clientWidth) * 0.99),
      nHeight = Math.ceil((window.innerHeight || 
                           document.documentElement.clientHeight || 
                           document.body.clientHeight) * 0.95),
      // nMin = Math.min(nWidth, nHeight),
      nLeft  = Math.max(20, Math.floor(Math.min(nWidth, nHeight) * 0.10)),
      nRight = Math.ceil(nWidth * 0.98),
      nTop   = Math.max(20, Math.floor(nHeight * 0.075)),
      nBot   = nHeight - nLeft, // Same as left side
      dWidth = nRight - nLeft,
      dHeight = nBot - nTop,
      subTickLength = Math.round(Math.min(nLeft, dWidth, dHeight) * 0.1),
      fontBase = "px Arial",
      fontLabelSize = Math.round(nLeft * 0.45),
      fontLabel = fontLabelSize + fontBase,
      titleFontSize = Math.ceil(nTop * 0.75),
      posTitle = Math.round(dWidth / 2) + nLeft,
      rhoTarget = 1,
      tAxis,
      yAxis,
      i,
      e, pop;

  if ((dWidth < 20) ||
      (dHeight < 20) || 
      (nLeft < 20) || 
      (nTop < 20) || 
      ((nHeight-nBot) < 20)) {
    myAlert("Canvas not sufficiently large, " + nWidth + "x" + nHeight +
            " d " + dWidth + "x" + dHeight + 
            " left " + nLeft + 
            " top " + nTop +
            " bot " + (nHeight-nBot));
  }

  ctx.canvas.width = nWidth;
  ctx.canvas.height = nHeight;

  // draw a box around the plot, defaults are black and 1 pixel line width
  ctx.strokeRect(nLeft, nTop, dWidth, dHeight);

  // Title at top of plot
  ctx.textAlign = 'center';
  ctx.textBaseline = 'middle';
  for (i = titleFontSize; i >= 5; --i) { // adjust font size to fit into width
    ctx.font = i + fontBase;
    e = ctx.measureText(d.title).width;
    if (e <= dWidth) {
      break;
    }
  } // for i
  ctx.fillText(d.title, posTitle, Math.round(nTop/2));

  // draw the y axis label
  ctx.font = fontLabel;
  ctx.textBaseline = 'top';
  rotText(ctx, d.ylabel + ' (' + d.units + ')', 
          0, Math.round(dHeight / 2) + nTop);

  tAxis = mktAxis(d.t, d.t0, nLeft, nRight, 0, nWidth, 
                  ppi, ctx, fontLabelSize, fontBase, rhoTarget);

  yAxis = mkyAxis(d.y, nBot, nTop, 
                  ((tAxis.labelPos[0]-tAxis.labelWidth[0]/2)<nLeft) ? nBot:nHeight, 
                  0, 
                  ppi, ctx, fontLabelSize, fontBase, rhoTarget);

  // Plot the data
  ctx.strokeStyle = 'blue';
  ctx.beginPath();
  ctx.moveTo(tAxis.xform(d.t[0]), yAxis.xform(d.y[0]));
  for (i = 1, e = d.t.length; i < e; ++i) {
    ctx.lineTo(tAxis.xform(d.t[i]), yAxis.xform(d.y[i]));
  }
  ctx.stroke();

  // Draw  ticks
  ctx.strokeStyle = '#808080'; // Grey
  ctx.beginPath();

  for (i = 0, e = tAxis.tickPos.length; i < e; ++i) { // t
    ctx.moveTo(tAxis.tickPos[i], nTop);
    ctx.lineTo(tAxis.tickPos[i], nBot);
  }

  for (i = 0, e = yAxis.tickPos.length; i < e; ++i) { // y
    ctx.moveTo(nLeft, yAxis.tickPos[i]);
    ctx.lineTo(nRight, yAxis.tickPos[i]);
  }

  ctx.stroke();

  // Draw y subticks
  ctx.strokeStyle = '#00ff00'; // Green
  ctx.beginPath();
  for (i = 0, e = yAxis.subPos.length; i < e; ++i) {
    ctx.moveTo(nLeft, yAxis.subPos[i]);
    ctx.lineTo(nLeft + subTickLength, yAxis.subPos[i]);
    ctx.moveTo(nRight, yAxis.subPos[i]);
    ctx.lineTo(nRight- subTickLength, yAxis.subPos[i]);
  }
  ctx.stroke();

  ctx.strokestyle = 'black';
  ctx.textBaseline = 'top';
  ctx.font = Math.min(tAxis.fontSize, yAxis.fontSize) + fontBase;

  for (i = 0, e = tAxis.label.length; i < e; ++i) {
    ctx.fillText(tAxis.label[i], tAxis.labelPos[i], nBot);
  }

  ctx.textBaseline = 'bottom';

  for (i = 0, e = yAxis.label.length; i < e; ++i) {
    rotText(ctx, yAxis.label[i], nLeft, yAxis.labelPos[i]);
  }

  pop = pInfo.ctx;
  pop.canvas.width = nWidth;
  pop.canvas.height = nHeight;
  pInfo.rDot = Math.max(5, Math.round(Math.min(dWidth, dHeight) * 0.01));
  pInfo.dotSize = pInfo.rDot * 2 + 2;
  pInfo.tAxis = tAxis;
  pInfo.yAxis = yAxis;
  pInfo.xYPos = nWidth;
  pInfo.heightY = Math.round(nTop / 2);
  pInfo.genOnFont = fontLabel;
  pInfo.txtX = Math.round(nWidth / 2);
  pInfo.txtY = nHeight - fontLabelSize;
  pInfo.txtLeft = 0;
  pInfo.txtWidth = nWidth;
  pInfo.txtTop = nBot;
  pInfo.txtHeight = nHeight - nBot;
  pop.font = fontLabel;
  pop.textAlign = 'center';
  pop.textBaseline = 'top';
  pop.fillText(pInfo.genOn, pInfo.txtX, pInfo.txtY);
} // drawData

function initPopInfo(d, canvas0, pop) {
  var now = new Date();

  pInfo = {ctx: pop,
           rect: canvas0.getBoundingClientRect().left,
           t: d.t,
           y: d.y,
           n: d.t.length - 1,
           units: d.units,
           genOn: 'Drawn ' +
                  (now.getMonth() + 1) + '/' +
                  now.getDate() + '/' +
                  format2(now.getFullYear() % 100) + ' ' +
                  now.getHours() + ':' +
                  format2(now.getMinutes())};
} // initPopInfo

function mktAxis(t, t0, lhs, rhs, lhsFrame, rhsFrame, ppi,
                 ctx, fontSize, fontBase, rho) {
  var min = t[t.length-1],
      max = t[0],
      date,
      info;
 
  if (min == max) {
    min -= 1800;
    max += 1800;
  }
 
  info = new axis(min, max, lhs, rhs, lhsFrame, rhsFrame, ppi,
                  ctx, fontSize, fontBase, rho);

  info.t0 = t0;

  info.mkDate = function(t) {return new Date((t + this.t0) * 1000);};

  info.formatDate = function(t) { // Turn t into a date string
    var d = this.mkDate(t);
    return format2(d.getMonth() + 1) + '/' + 
           format2(d.getDate()) + '/' +
           d.getFullYear();
  }; // formatDate

  info.format = function(t) { // Turn into a date string for mouseover
      var d = this.mkDate(t);
      return d.getHours() + ':' + format2(d.getMinutes()) +
             ' ' + (d.getMonth() + 1) + '/' + d.getDate() +
             '/' + format2(d.getFullYear() % 100);
    }; // format

  info.qZero = function(a,b) {return false;};

  info.firstTick = function(d) { // location of first tick >= min
      return this.round(Math.ceil(this.min / d) * d);
    };

  info.lastTick = function(d) { // location of first tick <= max
      return this.round(Math.floor(this.max / d) * d);
    };

  info.stride = function(t, d) {return Math.round(t + d);};

  if (info.range > (4 * 31 * 86400)) { // > 4months
    date = info.mkDate(max);
    info.max = date.getFullYear() * 12 + date.getMonth();
    date = info.mkDate(min);
    info.min = date.getFullYear() * 12 + date.getMonth() +
               (date.getDate() !== 1);
    info.range = info.max - info.min;
    info.toTime = function(m) {
      var d = new Date(Math.floor(m / 12), m % 12);
      return d.getTime() / 1000 - t0;
    };
    info.yform = function(m, lMin, lMax) { // Transform from months to pixels
      return Math.round((this.toTime(m)-min)/(max-min)*this.tWidth)+this.lhs;
    };
    info.firstTick = function(d) {
      return Math.ceil(this.min / d) * d;
    };
    info.lastTick = function(d) {
      return Math.floor(this.max / d) * d;
    };
    info.mkLabel = function(m, i) { return mktLabel(info.toTime(m), i, t0); };
    info.formats = [0,1,2,3];
    info.Q = [1, 2, 3, 4, 6];
  } else if (info.range > 4 * 60) { // > 4 minutes && <= 4 days
    if (info.range > 4 * 86400) { // > 4 days
      info.multiplier = 86400;
      info.formats = [10,11];
      info.Q = [1, 7, 2, 3, 4];
      info.roundType = 'day';
    } else if (info.range > 4 * 3600) { // > 4 hours
      info.multiplier = 3600;
      info.formats = [20,21];
      info.Q = [1, 6, 2, 4, 3];
      info.roundType = 'hour';
    } else if (info.range > 40 * 60) { // > 40 minutes
      info.multiplier = 600;
      info.formats = [30];
      info.Q = [1, 6, 2, 4, 3];
      info.roundType = 'min';
    } else { // > 4 minutes
      info.multiplier = 60;
      info.formats = [30];
      info.Q = [1, 5, 2, 4, 3];
      info.roundType = 'min';
    }

    info.sdate = ceilDate(min + t0, info.roundType).getTime() / 1000;
    info.min = 0;
    info.max = (floorDate(max + t0, info.roundType).getTime() / 1000 -
                info.sdate) / info.multiplier;
    info.range = info.max - info.min;
    info.yform = function(d, lMin, lMax) {
      var t = d * this.multiplier - t0 + this.sdate;
      return Math.round((t-min)/(max-min)*this.tWidth)+this.lhs;
    };
    info.mkLabel = function(d, i) {
        return mktLabel(d*this.multiplier + this.sdate, i, 0);
      };
  } else {
    info.mkLabel = function(t, i) {
        return mktLabel(t, i, t0);
      };
    info.formats = [30];
    info.Q = [1,6,2,5,4,3];
  }

  info.mkTicks();

  info.min = min;
  info.max = max;
  info.range = max - min;

  return info;
} // mktAxis

function mkyAxis(y, lhs, rhs, lhsFrame, rhsFrame, ppi, 
                 ctx, fontSize, fontBase, rho) {
  var min = Infinity,
      max = -Infinity,
      i, e,
      info;
      // ticks;

  for (i = 0, e = y.length; i < e; ++i) {
    min = y[i] < min ? y[i] : min;
    max = y[i] > max ? y[i] : max;
  }

  if ((max - min) < 1) {
    min -= 0.5;
    max += 0.5;
  }

  info = new axis(min, max, lhs, rhs, lhsFrame, rhsFrame, ppi,
                  ctx, fontSize, fontBase, rho);

  info.Q = [1, 5, 2, 2.5, 4, 3];
  info.formats = [0,1,2];
  info.mkLabel = function(x,i) {
    var ax = Math.abs(x);
    if (ax === 0) {
      return ['0', 1, false];
    }
    switch (i) {
      case 0: return [x.toPrecision(), (ax >= 1e-4) && (ax < 1e6), false];
      case 1: return [x.toExponential(), 0.25, false];
      case 2:
        if ((ax >= 1e3) && (ax < 1e6)) {
          return [(x * 1e-3).toFixed(0) + 'K', 0.75, true];
        }
        if ((ax >= 1e6) && (ax < 1e9)) {
          return [(x * 1e-6).toFixed(0) + 'M', 0.75, true];
        }
        if ((ax >= 1e9) && (ax < 1e12)) {
          return [(x * 1e-6).toFixed(0) + 'G', 0.75, true];
        }
      default: return [x, 0, false];
      }
    }; // mkLabel

  info.mkTicks();
  info.min = info.lMin;
  info.max = info.lMax;
  info.range = info.max - info.min;

  return info;
} // mkyAxis

function axis(min, max, lhs, rhs, lhsFrame, rhsFrame, ppi,
              ctx, fontSizeTarget, fontBase, rho) {
  this.min = min; // data min
  this.max = max; // data max
  this.range = max - min; // data range
  this.lhs = lhs; // axis left hand side in pixels
  this.rhs = rhs; // axis right hand side in pixels
  this.tWidth = rhs - lhs; // axis length in pixels
  this.lhsFrame = lhsFrame; // frame left hand side in pixels
  this.rhsFrame = rhsFrame; // frame right hand side in pixels

  this.ppi = ppi; // pixels per inch
  this.widthInches = Math.abs(this.tWidth) / ppi; // Axis length in inches

  this.ctx = ctx; // canvas context
  this.fontSizeTarget = fontSizeTarget;
  this.fontSizeMin = Math.max(5, Math.round(fontSizeTarget * 0.25));
  this.fontBase = fontBase; // font specifier suffix, "px Arial"

  this.rho = rho; // Target tick density in ticks/inch

  this.qLabels = true; // Do ticks have labels
  this.qTight = false; // are the axis ends at data min/max

  // Tick positions 
  this.tickPos = [];

  // Tick labels and positions
  this.label = [];
  this.labelPos = [];

  // Subtick positions and length
  this.subPos = [];
  this.subLength = Math.max(Math.round(0.01 * Math.abs(this.tWidth)), 1);

  this.yform = function(x, min, max) { // data n [min,max] to pixels
      return Math.round((x-min) / (max - min) * this.tWidth + this.lhs);
    };
  this.xform = function(x) { // data units to pixels
      return Math.round((x - this.min) / this.range * this.tWidth) + this.lhs;
    };
  this.iform = function(x) { // pixesl to data coordinates
      return (x - this.lhs) / this.tWidth * this.range + this.min;
    };

  this.round = function(x) { // Round to a reasonabl value
     return Math.round(x * 10000) / 10000;
    };

  this.firstTick = function(d) { // location of first tick <= min
      return this.round(Math.floor(this.min / d) * d);
    };

  this.lastTick = function(d) { // location of first tick >= max
      return this.round(Math.ceil(this.max / d) * d);
    };

  this.stride = function(x, d) { // x offset by d
      return this.round(x + d);
    };

  this.qZero = function(x, d) { // for a tick at x, and a step size of d, is zero a tick
    return this.round(Math.round(-x/d) * d + x) === 0;
  };

  this.mkTicks = function() {
    var info;
    this.nQm1 = this.Q.length;
    this.qDebug = false;
    info = mkTicks(this);
    this.fontSize = info.fs;
    this.ticks = info.ticks;
    this.tickPos = info.pos;
    this.label = info.label;
    this.labelPos = info.labelPos;
    this.labelWidth = info.width;
    this.lMin = info.lMin;
    this.lMax = info.lMax;
  };

  this.score = function(s, c, d, l) {
      return 0.2 * s + 0.25 * c + 0.5 * d + 0.05 * l;
    };

  this.simplicity = function(i, j, qZero) {
      return 1 - i / this.nQm1 - (j+1) + qZero;
    }; // simplicity

  this.density = function(lStep) {
      var ratio = (this.range / lStep / this.widthInches) / this.rho;
      return 2 - Math.max(ratio, 1/ratio);
    }; // density
  this.coverage = function(lMin, lMax) {
      var d0 = this.max - lMax,
          d1 = this.min - lMin,
          d2 = 0.1 * this.range;
      return !this.qTight ? 1 - (d0 * d0 + d1 * d1) / (d2 * d2) / 2 :
                            ((d0 !== 0) || (d1 !== 0) ? -Infinity : 1);
    }; // coverage
} // prototype for ticks


function mkTicks(info) {
  var best = {pos: [], label: [], labelPos: []},
      nQ = info.Q.length,
      // min = info.min,
      // max = info.max,
      stepNorm = Math.pow(10, Math.round(Math.log(info.range)/Math.LN10)-1),
      i, 
      j,
      k,
      offsetIndices = [],
      q,
      q0,
      key,
      qSeen = {},
      denom,
      lStep,
      simp,
      score,
      bestScore = -Infinity,
      // rho,
      // rhot = info.rho,
      dens,
      lMin,
      lMax,
      cov,
      leg;

  for (j = 0; j < 10; ++j) {
    offsetIndices.push(j);
    for (i = 0; i < nQ; ++i) {
      q = info.Q[i];
      denom = Math.pow(10, Math.floor(Math.log((j+1)*q)/Math.LN10));
      lStep = (j+1) * q / denom * stepNorm;
      for (k = 0; k <= j; ++k) {
        q0 = q * q * offsetIndices[k] / denom;
        key = lStep + ":" + q0;
        if (key in qSeen) { // Already seen this combination
          continue;
        }
        qSeen[key] = true;
        simp = info.simplicity(i, j, info.qZero(q0, lStep));
        score = info.score(simp, 1, 1, 1);

        if (score < bestScore) { // can't be better
          break;
        }

        dens = info.density(lStep);
        score = info.score(simp, 1, dens, 1);

        if (score < bestScore) { // can't be better
          break;
        }
  
        lMin = info.qTight ? info.min : info.firstTick(lStep);
        lMax = info.qTight ? info.max : info.lastTick(lStep);

        cov = info.coverage(lMin, lMax);
        score = info.score(simp, cov, dens, 1);

        if (score < bestScore) { // can't be better
          break;
        }

        leg = legibility(lMin, lMax, lStep, info);
        score = info.score(simp, cov, dens, leg.score);
        if (score < bestScore) { // can't be better
          break;
        }
        bestScore = score;
        best = leg;
        best.lMin = lMin;
        best.lMax = lMax;
      } // for k
    } // for i
  } // for j

  return best;
} // mkTicks

function legibility(min, max, delta, info) {
  var tInfo = {fs: info.fontSizeTarget, pos: [], label: [], labelPos: []},
      ctx = info.ctx,
      fontBase = info.fontBase,
      fs, 
      fsTgt = info.fontSizeTarget,
      fsMin = info.fontSizeMin, fsScore,
      iFormat, 
      j, je,
      tick,
      ticks = [],
      labels,
      labelPos,
      pos = [],
      i,
      nTicks,
      lInfo,
      qZeroExtended = false,
      qSkip = false,
      qDuplicate,
      qLabelValues,
      netScore,
      // distScore, 
      score,
      bestScore = -Infinity,
      distance,
      overlap,
      width;

  for (tick = min; tick <= max; tick = info.stride(tick, delta)) {
    ticks.push(tick);
    pos.push(info.yform(tick, min, max));
    qZeroExtended |= tick === 0;
  }

  nTicks = ticks.length;
  tInfo.pos = pos;
  tInfo.ticks = ticks;

  if (!info.qLabels) {
    tInfo.score = 1;
    return tInfo;
  }

  for (j = 0, je = info.formats.length; j < je; ++j) {
    iFormat = info.formats[j];
    labels = [];
    qLabelValues = {};
    qDuplicate = false;
    netScore = 0; 
    for (i = 0; i < nTicks; ++i) {
      lInfo = info.mkLabel(ticks[i], iFormat);
      labels.push(lInfo[0]);
      netScore += lInfo[1];
      qSkip |= lInfo[2];
      qDuplicate |= lInfo[0] in qLabelValues;
      if (qDuplicate) { // No need to do anything else
        break;
      }
      qLabelValues[lInfo[0]] = true;
    } // for i
    if (qSkip && qDuplicate) { // rest of formats will cause duplicates
      break;
    }

    netScore *= 0.9 / nTicks;
    netScore += 0.1 * qZeroExtended;
    score = (netScore + 3) / 4;
    if (score < bestScore) {
      continue;
    }

    for (fs = fsTgt; fs >= fsMin; --fs) {
      fsScore = (fs >= fsTgt) ? 1 : 0.8 * (fs - fsMin) / (fsTgt - fsMin);
      score = (netScore + fsScore + 2) / 4;
      if (score < bestScore) {
        break;
      }

      ctx.font = fs + fontBase;
      overlap = Infinity;
      width = [];
      labelPos = [];
      for (i = 0; i < nTicks; ++i) {
        width.push(ctx.measureText(labels[i]).width);
        labelPos.push(pos[i]);
        if (i === 0) { // first tick, adjust to be inside the plot axis
          distance = ((info.rhsFrame > info.lhsFrame) ? 1 : -1) *
                     Math.min(Math.round(Math.abs(labelPos[i]-info.lhsFrame) - 
                                         width[i]/2), 
                              0);
          labelPos[i] -= distance;
        }
        if (i == (nTicks - 1)) {
          distance = ((info.rhsFrame > info.lhsFrame) ? 1 : -1) *
                     Math.min(Math.round(Math.abs(labelPos[i]-info.rhsFrame) - 
                                         width[i]/2), 
                              0);
          labelPos[i] += distance;
        }
        if (i > 0) { // check overlap between labels
          distance = Math.round(Math.abs(labelPos[i-1] - labelPos[i]) -
                                (width[i-1] + width[i]) / 2);
          overlap = Math.min(overlap, overlapScore(distance, fs));
        }
      } // for i

      score = (netScore + fsScore + overlap + 1) / 4;
      if (score < bestScore) {
        continue;
      }

      bestScore = score;
      tInfo.score = score;
      tInfo.fs = fs;
      tInfo.label = labels;
      tInfo.labelPos = labelPos;
      tInfo.width = width;
    } // for fs
  } // for iFormat

  return tInfo;
}

function mktLabel(t, i, t0) {
  var d = new Date((t + t0) * 1000),
      monthNames = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
                    'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];

  switch (i) {
    case 0: return [monthNames[d.getMonth()] + '-' + d.getFullYear(), 1, true];
    case 1: return [monthNames[d.getMonth()] + '-' + 
                    format2(d.getFullYear() % 100), 0.9, true];
    case 2: return [(d.getMonth() + 1) + '/' + d.getFullYear(), 0.8, true];
    case 3: return [(d.getMonth() + 1) + '/' + format2(d.getFullYear() % 100), 
                    0.7, true];
    case 10: return [monthNames[d.getMonth()] + '-' + d.getDate(), 1, true];
    case 11: return [(d.getMonth() + 1) + '/' + d.getDate(), 0.9, true];
    case 20: return [monthNames[d.getMonth()] + '-' + d.getDate() +
                     ' ' + d.getHours() + ':' + format2(d.getMinutes()), 
                     1, true];
    case 21: return [(d.getMonth() + 1) + '/' + d.getDate() +
                     ' ' + d.getHours() + ':' + format2(d.getMinutes()), 
                     0.9, true];
    case 30: return [d.getHours() + ':' + format2(d.getMinutes()), 1, true];
    default: return [(d.getMonth() + 1) + '/' + d.getDate() + '/' + 
                     d.getFullyear() + ' ' +
                     d.getHours() + ':' + format2(d.getMinutes()), 1, true];
  }
} // mktLabel

function format2(x) {
  return (x < 10) ? '0' + x.toString() : x.toString();
} // format2

function overlapScore(dist, fs) {
  var threshold = 0.5 * fs; // 1.5 is not enough
  return (dist >= threshold) ? 1 :
         (dist > 0) ? (2 - threshold / dist) :
         -Infinity;
} // overlapScore

function floorDate(t, flg) {
  var d = new Date(t * 1000);
  switch (flg) {
    case 'mon':   return new Date(d.getFullYear(), d.getMonth());
    case 'day':   return new Date(d.getFullYear(), d.getMonth(), d.getDate());
    case 'hour':  return new Date(d.getFullYear(), d.getMonth(), 
                                  d.getDate(), d.getHours());
    default: return d;
  }
} // floorDate

function ceilDate(t, flg) {
  var d = new Date(t * 1000);
  switch (flg) {
    case 'mon': return new Date(d.getFullYear(), 
                                d.getMonth() + 
                                ((d.getDate() !== 1) ||
                                 (d.getHours() !== 0) ||
                                 (d.getMinutes() !== 0) ||
                                 (d.getSeconds() !== 0) ||
                                 (d.getMilliseconds() !== 0)));
    case 'day': return new Date(d.getFullYear(), 
                                d.getMonth(),
                                d.getDate() + 
                                ((d.getHours() !== 0) ||
                                 (d.getMinutes() !== 0) ||
                                 (d.getSeconds() !== 0) ||
                                 (d.getMilliseconds() !== 0)));
    case 'hour':return new Date(d.getFullYear(), 
                                d.getMonth(),
                                d.getDate(),
                                d.getHours() +
                                ((d.getMinutes() !== 0) ||
                                 (d.getSeconds() !== 0) ||
                                 (d.getMilliseconds() !== 0)));
    default: return d;
  }
} // ceilDate

function myMouse(e) {
  var tAxis = pInfo.tAxis,
      cX = e.clientX || e.touches[0].clientX,
      x = tAxis.iform(cX - pInfo.rect),
      t = pInfo.t,
      y = pInfo.y,
      n = pInfo.n,
      ctx = pInfo.ctx,
      iStart = 0, 
      iStop = n,
      iMid,
      dist0,
      txt;

  while (iStart < iStop) {
    iMid = Math.floor((iStart + iStop) / 2); // Mid point
    if (t[iMid] > x) {
      iStart = iMid + 1;
    } else {
      iStop = iMid - 1;
    }
  }

  dist0 = Math.abs(t[iStart] - x);
  iStart = iStart -
           (Math.abs(t[iStart - (iStart > 0)] - x) < dist0) +
           (Math.abs(t[iStart + (iStart < n)] - x) < dist0);

  if ('timeout' in pInfo) {
    if (pInfo.iPrev == iStart) { // no change
      return;
    }
    clearTimeout(pInfo.timeout);
    delete(pInfo.timeout);
  }

  if ('dotLeft' in pInfo) {
    ctx.clearRect(pInfo.dotLeft, pInfo.dotTop, pInfo.dotSize, pInfo.dotSize);
  }

  pInfo.iPrev = iStart;
  pInfo.tDot = tAxis.xform(t[iStart]);
  pInfo.yDot = pInfo.yAxis.xform(y[iStart]);
  pInfo.dotLeft = pInfo.tDot - pInfo.rDot - 1;
  pInfo.dotTop = pInfo.yDot - pInfo.rDot - 1;

  ctx.beginPath();
  ctx.arc(pInfo.tDot, pInfo.yDot, pInfo.rDot, 0, 2 * Math.PI);
  ctx.fill();
  ctx.stroke();

  txt = y[iStart] + '(' + pInfo.units + ') ' + tAxis.format(t[iStart]);
  ctx.clearRect(pInfo.txtLeft, pInfo.txtTop, pInfo.txtWidth, pInfo.txtHeight);
  ctx.fillText(txt, pInfo.txtX, pInfo.txtY);

  pInfo.timeout = setTimeout(function() {
      var ctx = pInfo.ctx;
      clearTimeout(pInfo.timeout);
      delete(pInfo.timeout);
      ctx.clearRect(pInfo.dotLeft, pInfo.dotTop, 
                    pInfo.dotSize, pInfo.dotSize);
      ctx.clearRect(pInfo.txtLeft, pInfo.txtTop, 
                    pInfo.txtWidth, pInfo.txtHeight);
      ctx.fillText(pInfo.genOn,pInfo.txtX, pInfo.txtY);
    }, 10000); // 10 seconds
} // myMouse

function rotText(ctx, txt, x, y) {
  ctx.save();
  ctx.translate(x, y);
  ctx.rotate(-Math.PI / 2);
  ctx.fillText(txt, 0, 0);
  ctx.restore();
} // rotText

function updateForm(d) {
  var x = document.getElementById("frm1"),
      n = d.t.length - 1,
      date;

  if (x.elements[0].type == "date") { // date type is supported
    x.elements[0].valueAsDate = new Date((d.t[n] + d.t0) * 1000);
    x.elements[1].valueAsDate = new Date((d.t[0] + d.t0) * 1000);
  } else {  // date type is not supported
    date = new Date((d.t[n] + d.t0) * 1000);
    x.elements[0].value = (date.getMonth() + 1) + 
                          '-' + date.getDate() + 
                          '-' + date.getFullYear();
    date = new Date((d.t[0] + d.t0) * 1000);
    x.elements[1].value = (date.getMonth() + 1) + 
                          '-' + date.getDate() + 
                          '-' + date.getFullYear();
  }

  x.elements[0].focus();
}

function myAlert(msg) {
  alert(msg);
  throw msg;
}
