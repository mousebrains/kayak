'use strict';

var pInfo = {}; // Plot object information

function lev(d) {
  var div = mkDiv(),
      tbl = document.createElement("table"),
      thead = tbl.createTHead(), // Header rows
      tbody = tbl.createTBody(), // Body rows
      tfoot = tbl.createTFoot(), // Footer rows
      qFlow = "flow" in d,
      qGauge = "gauge" in d,
      qTemp = "temperature" in d,
      qClass = "class" in d,
      now = new Date,
      tOld = now / 1000 - 2 * 86400,
      colVars = ["indices", "t"],
      sortDir = [-1, -1],
      contents = [],
      header = [],
      row, 
      cell, 
      hash, 
      arrow, 
      txt, 
      i, 
      n = d.t.length;

  decodeData(d, "t"); // Expand back to seconds

  var sorter = function(index) {
    var indices = [],
        x = d[colVars[index]],
        sgn = sortDir[index],
        arrow = sgn < 0 ? ' &uarr;' : ' &darr;',
        n = contents.length,
        i, e;

    for (i = 0, e = n; i < e; ++i) {
      indices.push(i); // Build a sequence of indices
    }

    for (i = 0, e = header.length; i < e; ++i) { // Set arrow in appropriate col and direction
      thead.rows[0].cells[i].innerHTML = header[i] + (i == index ? arrow : "");
      tfoot.rows[0].cells[i].innerHTML = thead.rows[0].cells[i].innerHTML;
    }

    if (typeof x[0] == "string") {
      indices.sort(function(i,j) {
          return x[sgn > 0 ? i : j].localeCompare(x[sgn > 0 ? j : i]);
        });
    } else { // numeric sort
      indices.sort(function(i,j) {
          var aa = x[i] == undefined ? 0 : x[i],
              bb = x[j] == undefined ? 0 : x[j];
          return sgn * (aa - bb);
        });
    }

    sortDir[index] = -sortDir[index];

    for (i = 0, e = n; i < e; ++i) {
      tbody.rows[i].outerHTML = contents[indices[i]];
    }
  }

  var setHdr = function(cell, index, title) {
    header.push(title);
    cell.innerHTML = title + (index == 0 ? " &darr;" : "");
    cell.className = "hdr";
    cell.onclick = function() {sorter(index);}
    return cell;
  }
   
  row = thead.insertRow(-1);
  i = -1;
  setHdr(row.insertCell(-1), ++i, "Name");
  setHdr(row.insertCell(-1), ++i, "Date");

  if (qFlow) {
    setHdr(row.insertCell(-1), ++i, "Flow<br>CFS");
    colVars.push("flow");
    sortDir.push(-1);
  }

  if (qGauge) {
    setHdr(row.insertCell(-1), ++i, "Height<br>Feet");
    colVars.push("gauge");
    sortDir.push(1);
  }
  
  if (qTemp) {
    setHdr(row.insertCell(-1), ++i, "Temp<br>F");
    colVars.push("temperature");
    sortDir.push(1);
  }

  if (qClass) {
    setHdr(row.insertCell(-1), ++i, "Class");
    colVars.push("grade");
    sortDir.push(1);
  }

  var setVal = function(cell, val, hash, level, delta, type) {
    var names = ["lo", "ok", "hi"];
    if (val != undefined) {
      cell.innerHTML = "<a href='?o=p&amp;h=" + hash + "&amp;t=" + type + "'>" + 
                       ((delta == undefined) ? "" : 
                        ("<span class='lev" + Math.abs(delta) + "'>&" +
                         (delta < 0 ? "d" : "u") + "arr;</span>")) +
                       val.toLocaleString() + "</a>";
      if (level != undefined) cell.className = names[level - 1];
    }
  }

  d.indices = [];
  d.grade = [];

  for (i = 0; i < n; ++i) {
    d.indices.push(i);
    hash = d.hash[i];

    arrow = d.level[i] ? 
            ("<span class='lev" + Math.abs(d.level[i]) + "'>&" +
             (d.level[i] < 0 ? "d" : "u") + "arr;</span>") :
            "";

    row = tbody.insertRow(-1);

    cell = row.insertCell(-1)
    cell.innerHTML = "<a href='?o=i&h=" + hash +"'>" + d.name[i] + "</a>";
    cell.className = "hdr";

    cell = row.insertCell(-1);
    cell.innerHTML = t2str(d.t[i], false);
    d.t[i] < tOld && (cell.className = "old");

    qFlow && setVal(row.insertCell(-1), d.flow[i], hash, d.level[i], d.flowDelta[i], "f");
    qGauge && setVal(row.insertCell(-1), d.gauge[i], hash, d.level[i], d.gaugeDelta[i], "g");
    qTemp && setVal(row.insertCell(-1), d.temperature[i], hash, d.level[i], d.temperatureDelta[i], "t");
    if (qClass) {
      txt = d.class[i];
      cell = row.insertCell(-1);
      cell.innerHTML = txt;
      cell.className = "hdr";
      d.grade.push(txt.search("VI") >= 0 ? 6 :
                   txt.search("IV") >= 0 ? 4 :
                   txt.search("V")  >= 0 ? 5 :
                   txt.search("III")>= 0 ? 3 :
                   txt.search("II") >= 0 ? 2 :
                   txt.search("I")  >= 0 ? 1 :
                   0);
    }

    contents.push(row.innerHTML);
  }
 
  tfoot.appendChild(thead.rows[0].cloneNode(true));
  i = 2 + qFlow + qGauge + qTemp + qClass;
  tfoot.insertRow(-1).innerHTML = "<th class='lo' colspan='" + i + "'>Low</th>";
  tfoot.insertRow(-1).innerHTML = "<th class='ok' colspan='" + i + "'>Okay</th>";
  tfoot.insertRow(-1).innerHTML = "<th class='hi' colspan='" + i + "'>High</th>";

  div.appendChild(tbl); 

  var h1 = document.createElement("h1");
  h1.innerHTML = "Generated " + now;
  div.appendChild(h1);

  var dl = document.createElement("DL");
  var dd;
  for (i = 1; i <= 10; ++i) {
    dd = document.createElement("DD");
    dd.innerHTML = "<span class='lev" + i + "'>&uarr;&darr;</span> Changing between " +
                   (i-1) + ".5% and " + i + ".5% per hour";
    dl.appendChild(dd);
  }
  dd = document.createElement("DD");
  dd.innerHTML = "<span class='lev11'>&uarr;&darr;</span> Changing more than 10.5% per hour";
  dl.appendChild(dd);

  div.appendChild(dl); 
}

function tbl(d) {
  initTblPlt(d, "Table");
  mkTbl();
}

function mkTbl() {
  var names = ["t", "y", "y1", "y2", "y3", "y4"],
      div = mkDiv(), // Where to put everything
      tbl = document.createElement("table"), // Table to put stuff in
      thead = tbl.createTHead(), // Header rows
      tbody = tbl.createTBody(), // Body rows
      tfoot = tbl.createTFoot(), // Footer rows
      h1 = document.createElement("h1"), // Title at top
      infoStr = buildQueryString(["h"]),
      qLow  = "low" in pInfo,
      qHigh = "high" in pInfo,
      qOkay = qLow && qHigh,
      sortDir = [],
      header = [],
      cols = [],
      contents = [],
      var1, var2, var3,
      row, i, e, j, je, cell, val;

  for (i = 0, e = names.length; i < e; ++i) {
    var1 = names[i];
    if (var1 in pInfo) {
      var2 = var1 + "label";
      var3 = var1 + "units";
      cols.push(var1);
      header.push((var2 in pInfo ? pInfo[var2] : "") + 
                  (var3 in pInfo ? ("<br>" + pInfo[var3]) : ""));
      sortDir.push(1);
    }
  }
 
  h1.innerHTML = '<a href="?o=i&amp;' + infoStr + '">' + pInfo.title + '</a>';

  div.appendChild(h1);

  tbl.setAttribute("id", "myTable");

  var sorter = function(col) {
    var a = [],
        y = pInfo[cols[col]],
        n = y.length,
        qString = typeof y[0] === 'string',
        arrow = sortDir[col] < 0 ? ' &uarr;' : ' &darr;',
        i, e;

    for (i = 0, e = header.length; i < e; ++i) { // Set arrow in appropriate col and direction
      thead.rows[0].cells[i].innerHTML = header[i] + (i == col ? arrow : "");
      tfoot.rows[0].cells[i].innerHTML = thead.rows[0].cells[i].innerHTML;
    }

    for (i = 0; i < n; ++i) {  // array of indices, 0...n-1
      a.push(i); 
    }

    if (qString) {
      a.sort(function(i,j) {
        var aa = y[sortDir[col] > 0 ? i : j].toLowerCase(),
            bb = y[sortDir[col] > 0 ? j : i].toLowerCase();
        return aa < bb ? -1 : aa > bb ? 1 : 0;
      });
    } else { // numeric sort
      a.sort(function(i,j) {return sortDir[col] * (y[i] - y[j])});
    }
    sortDir[col] = -sortDir[col];
    for (i = 0; i < n; ++i) {
      tbody.rows[i].outerHTML = contents[n - a[i] - 1];
    }
  }

  var setHdr = function(cell, index, title) {
    cell.innerHTML = title;
    cell.className = "hdr";
    cell.onclick = function() {sorter(index);}
    return cell;
  }
   
  row = thead.insertRow(-1);

  for (i = 0, e = cols.length; i < e; ++i) {
    setHdr(row.insertCell(-1), i, header[i] + (i == 0 ? " &uarr;" : ""));
  }

  var qClass = false;

  for (i = pInfo[cols[0]].length-1; i >= 0; --i) {
    row = tbody.insertRow(-1);
    for (j = 0, je = cols.length; j < je; ++j) { // Insert data
      val = pInfo[cols[j]][i];
      cell = row.insertCell(-1);
      if (j == 0) { // time
        cell.innerHTML = t2str(val, true);
      } else { // not time
        cell.innerHTML = val.toLocaleString(); // Comma deliminated
      }
      if (j == 1) { // Check high/low
        if (qHigh && pInfo.high < val) {
          cell.className = "hi";
          qClass = true;
        } else if (qLow && pInfo.low > val) {
          cell.className = "lo";
          qClass = true;
        } else if (qOkay) {
          cell.className = "ok";
          qClass = true;
        }
      }
    }
    contents.push(row.outerHTML);
  }

  tfoot.appendChild(thead.rows[0].cloneNode(true));

  if (qClass) {
      var labFun = function(label, cname) {
        var cell = tfoot.insertRow(-1).insertCell(-1);
        cell.innerHTML = label;
        cell.className = cname + " hdr";
        cell.setAttribute("colspan", cols.length);
      }

    labFun("High", "hi");
    labFun("Okay", "ok");
    labFun("Low", "lo");
  }

  div.appendChild(tbl);
  div.appendChild(h1.cloneNode(true));
  
  var frm = document.getElementById("frm").cloneNode(true);
  frm.id = "frm2";
  updateForm(frm);
  div.appendChild(frm);
} // tbl

function plt(d) {
  initTblPlt(d, "Plot");
  mkPlt();
}

function mkPlt() {
  if (!pInfo.qCanvas) { // This browser does not support canvas, so fall back to image
    noCanvas(); 
    return;
  }

  initPlt();

  draw();
  window.onresize = function(e) {draw();} // when window is resized redraw
}

function setupCanvas() {
  var ctx = pInfo.ctx,
      pop = pInfo.pop,
      sWidth = window.innerWidth || 
               document.documentElement.clientWidth ||  
               document.body.clientWidth,
      sHeight = window.innerHeight ||
                document.documentElement.clientHeight || 
                document.body.clientHeight,
      width = sWidth,
      height = sHeight;

  height -= ctx.canvas.offsetTop; // Take off part above the canvas

  ctx.canvas.width = width;    
  pop.canvas.width = width;    
  ctx.canvas.height = height;    
  pop.canvas.height = height;    

  pInfo.left = ctx.canvas.getBoundingClientRect().left;

  return {width: width, height: height};
} // setupCanvas

function draw() {
  var ctx = pInfo.ctx,
      pop = pInfo.pop,
      sInfo = setupCanvas(),
      nWidth = sInfo.width,
      nHeight = sInfo.height,
      nLeft  = Math.max(20, Math.floor(Math.min(nWidth, nHeight) * 0.1)), // left width
      nRight = Math.ceil(nWidth - Math.max(10, nWidth * 0.02)), // Width on right side
      nTop   = Math.max(20, Math.floor(nHeight * 0.075)), // Height at top
      nBot   = nHeight - nLeft, // Same as left side
      dWidth = nRight - nLeft, // Plot box width
      dHeight= nBot - nTop, // Plot box height
      fontSuffix = pInfo.font, // Font suffixe "px Arial" or such
      fontSizeTitle = Math.ceil(nTop * 0.75),
      fontLabelSize = Math.round(nLeft * 0.45),
      fontLabel = fontLabelSize + fontSuffix,
      rhoTarget = 1, // 1 tick/inch
      taxis = mktAxis(nLeft, nRight, 0, nWidth, fontLabelSize, rhoTarget),
      yaxis = mkyAxis(nBot, nTop, 
                      Math.round(taxis.labelPos[0] - taxis.widths[0]/2) < nLeft ? nBot : nHeight, 
                      0, taxis.fs, rhoTarget),
      extras = [["low", "DarkOrange"], ["high", "Red"], 
                ["lowOptimal", "ForestGreen"], ["highOptimal", "ForestGreen"]],
      i, e;

  pInfo.taxis = taxis;
  pInfo.yaxis = yaxis;

  if ((dWidth < 20) || (dHeight < 20)) {
    myAlert("Canvas not sufficiently large, " + nWidth + "x" + nHeight +
            " delta " + dWidth + "x" + dHeight);
  }

  pInfo.txt.x = Math.round((nRight + nLeft) / 2);
  pInfo.txt.y = nHeight;
  pInfo.txt.left = 0;
  pInfo.txt.width = nWidth;
  pInfo.txt.top = nBot;
  pInfo.txt.height = nHeight - nBot;

  ctx.strokeRect(nLeft, nTop, dWidth, dHeight); // Plot box

  // Title at top
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.font = fontSizeTitle + fontSuffix;
  for (i = fontSizeTitle; 
       (i >= 5) && (ctx.measureText(pInfo.title).width > dWidth); --i) { // Adjust font to fit box
    ctx.font = i + fontSuffix;
  }
  ctx.fillText(pInfo.title, pInfo.txt.x, Math.round(nTop / 2));
  
  // y Axis label 
  ctx.font = fontLabel;
  ctx.textBaseline = "top";
  rotText(ctx, pInfo.ylabel + " (" + pInfo.yunits + ")", 0, Math.round((nBot + nTop)/2));

  // Draw axis labels
  ctx.font = Math.min(taxis.fs, yaxis.fs) + fontSuffix;
  ctx.textBaseline = "top";
  for (i = 0, e = taxis.label.length; i < e; ++i) { // t axis labels
    ctx.fillText(taxis.label[i], taxis.labelPos[i], nBot);
  }
  ctx.textBaseline = "bottom";
  for (i = 0, e = yaxis.label.length; i < e; ++i) { // y axis labels
    rotText(ctx, yaxis.label[i], nLeft, yaxis.labelPos[i]);
  }

  // Draw  axis ticks 
  ctx.strokeStyle = '#808080'; // Grey
  ctx.beginPath();
  for (i = 0, e = taxis.pos.length; i < e; ++i) { // t axis ticks
    ctx.moveTo(taxis.pos[i], nBot);
    ctx.lineTo(taxis.pos[i], nTop);
  } // for i yaxis
  for (i = 0, e = yaxis.pos.length; i < e; ++i) { // y axis ticks
    ctx.moveTo(nLeft, yaxis.pos[i]);
    ctx.lineTo(nRight,yaxis.pos[i]);
  } // for i yaxis
  ctx.stroke();

  // Draw data
  ctx.strokeStyle = 'blue';
  ctx.beginPath();
  for (i = 0, e = pInfo.t.length; i < e; ++i) {
    if (i === 0) {
      ctx.moveTo(taxis.xform(pInfo.t[i]), yaxis.xform(pInfo.y[i]));
    } else {
      ctx.lineTo(taxis.xform(pInfo.t[i]), yaxis.xform(pInfo.y[i]));
    }
  } // for i
  ctx.stroke();

  for (i = 0, e = extras.length; i < e; ++i) {
    if ((extras[i][0] in pInfo) && 
        (pInfo[extras[i][0]] >= yaxis.lhs)  &&
        (pInfo[extras[i][0]] <= yaxis.rhs)) {
      ctx.strokeStyle = extras[i][1];
      ctx.beginPath();
      ctx.moveTo(nLeft,  yaxis.xform(pInfo[extras[i][0]]));
      ctx.lineTo(nRight, yaxis.xform(pInfo[extras[i][0]]));
      ctx.stroke();
    }
  }

  pop.font = fontLabel; 
  pop.textAlign = "center";
  pop.textBaseline = "bottom";
  pop.fillText(pInfo.genOn, pInfo.txt.x, pInfo.txt.y);
}

function mkyAxis(lhsFrame, rhsFrame, lhsBox, rhsBox, fsMax, rhoTgt) {
  var min = pInfo.yMin,
      max = pInfo.yMax,
      ax;

  if (min == max) {
    min -= 0.5;
    max += 0.5;
  }

  ax = new axis(min, max, lhsFrame, rhsFrame, lhsBox, rhsBox, fsMax, rhoTgt);
  return ax.mkTicks();
} // mkyAxis

function mktAxis(lhsFrame, rhsFrame, lhsBox, rhsBox, fsMax, rhoTgt) {
  var min = pInfo.tMin,
      max = pInfo.tMax,
      range = max - min,
      etime,
      ax,
      best,
      qForward = false;

  if (min == max) {
    min -= 1800;
    max += 1800;
  }

  ax = new axis(min, max, lhsFrame, rhsFrame, lhsBox, rhsBox, fsMax, rhoTgt);

  ax.mkTime = function(x) {
      var date = new Date(this.t0), // copy
          wholeMonths,
          weeks;

      if (this.timeNorm > 0) { // simple normalization
        date.setSeconds(date.getSeconds() + (x * this.timeNorm));
        return date;
      }
      // step by months
      wholeMonths = Math.floor(Math.abs(x)); // Months to step by
      weeks = Math.round((Math.abs(x) - wholeMonths) * 4); // weeks to step by
      date.setMonth(date.getMonth() + (x < 0 ? -1 : 1) * wholeMonths);
      date.setDate(date.getDate() + (x < 0 ? -1 : 1) * weeks * 7); 
      date.setHours(0);
      date.setMinutes(0);
      date.setSeconds(0);
      return date;
    } // mkTime


  ax.timeNorm = 1;
  ax.t0 = new Date(min * 1000);
  qForward = (ax.t0.getMilliseconds() != 0) || (ax.t0.getSeconds() !== 0); 
  ax.t0.setMilliseconds(0);
  ax.t0.setSeconds(0);

  if (ax.range > (4 * 31 * 86400)) { // > 4 months
    ax.timeNorm = -1;
    ax.Q = [1, 2, 3, 4, 6];
    qForward |= (ax.t0.getMinutes() != 0) || (ax.t0.getHours() != 0) || (ax.getDate() != 1);
    ax.t0.setMinutes(0);
    ax.t0.setHours(0);
    ax.t0.setDate(1);
    qForward && ax.t0.setMonth(ax.t0.getMonth() + qForward); // round forwards, if needed

    ax.lhs = (1000 * min - ax.t0.getTime()) / (ax.t0 - ax.mkTime(-1));

    etime = new Date(max * 1000);
    ax.rhs = (etime.getFullYear() - ax.t0.getFullYear()) * 12 +
             (etime.getMonth() - ax.t0.getMonth());
    ax.rhs += (etime - ax.mkTime(ax.rhs)) / (ax.mkTime(ax.rhs + 1) - ax.mkTime(ax.rhs));

  } else { // <= 4 months so use time scaling
    if (ax.range > (4 * 86400)) { // (4 days, 4 months]
      ax.Q = [1, 7, 2, 3, 4];
      ax.timeNorm = 86400;
      ax.formats = [10, 11];
      qForward |= (ax.t0.getMinutes() != 0) || (ax.t0.getHours() != 0);
      ax.t0.setHours(0); 
      ax.t0.setMinutes(0); 
      qForward && ax.t0.setDate(ax.t0.getDate() + qForward); // round forwards, if needed
    } else if (ax.range > (4 * 3600)) { // (4 hours, 4 days]
      ax.Q = [1, 6, 2, 4, 3];
      ax.timeNorm = 3600;
      ax.formats = [20, 21, 22, 23];
      qForward |= ax.t0.getMinutes() != 0;
      ax.t0.setMinutes(0); 
      qForward && ax.t0.setHours(ax.t0.getHours() + qForward); // round forwards, if needed
    } else { // [0, 4 hours] We know minimum range is 1 hour from above
      ax.Q = [1, 6, 2, 4, 3];
      ax.timeNorm = 3600;
      ax.formats = [30];
      qForward |= (ax.t0.getMinutes() % 10) != 0;
      ax.t0.setMinutes(Math.ceil(ax.t0.getMinutes() / 10) * 10); 
    }
    ax.lhs = (min - ax.t0.getTime() / 1000) / ax.timeNorm;
    etime = new Date(max * 1000);
    ax.rhs = (etime - ax.t0) / 1000 / ax.timeNorm;
  }
  ax.delta = ax.rhs - ax.lhs;
  ax.range = ax.delta;
  ax.qTight = true; // keep tight axis

  ax.qZero = function(a,b) {return false;}
  ax.firstTick = function(d) {return this.round(Math.ceil(this.lhs/d)*d);}
  ax.lastTick = function(d) {return this.round(Math.floor(this.rhs/d)*d);}

  ax.months = ["Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"];

  ax.mkLabel = function(x, i) {
      var date = this.mkTime(x),
          wholeMonths,
          weeks,
          months = this.months;

      switch (i) {
        case 0: return {label: months[date.getMonth()] + "-" + date.getFullYear(), 
                        score: 1, qSkip: true};
        case 1: return {label: months[date.getMonth()] + "-" + format2(date.getFullYear() % 100), 
                        score: 0.9, qSkip: true};
        case 2: return {label: (date.getMonth()+1) + "/" + date.getFullYear(),
                        score: 0.8, qSkip: true};
        case 3: return {label: (date.getMonth()+1) + "/" + format2(date.getFullYear() % 100),
                        score: 0.7, qSkip: true};
        case 10: return {label: months[date.getMonth()] + "-" + date.getDate(),
                         score: 1, qSkip: true};
        case 11: return {label: (date.getMonth()+1) + "/" + date.getDate(),
                         score: 0.9, qSkip: true};
        case 20: return {label: months[date.getMonth()] + "-" + date.getDate() + " " +
                                date.getHours() + ":" + format2(date.getMinutes()),
                         score: 1, qSkip: true};
        case 21: return {label: (date.getMonth()+1) + "/" + date.getDate() + " " +
                                date.getHours() + ":" + format2(date.getMinutes()),
                         score: 0.9, qSkip: true};
        case 22: return {label: date.getDate() + " " + date.getHours() + ":" + 
                                format2(date.getMinutes()),
                         score: 0.8, qSkip: true};
        case 23: return {label: date.getDate() + " " + date.getHours(),
                         score: 0.7, qSkip: true};
        case 30: return {label: date.getHours() + ":" + format2(date.getMinutes()),
                         score: 1, qSkip: true};
      }
console.log("mkLabel time unrecognized code", i);
console.log(this);
throw("mkLabel unrecognized code " + i);
      return date.toDateString();
    } // mkLabel

  best = ax.mkTicks();
  best.lhs = min;
  best.rhs = max;
  best.delta = max - min;
  return best;
} // mktAxis

function axis(lhs, rhs, lhsFrame, rhsFrame, lhsBox, rhsBox, fsMax, rhoTgt) {
  this.lhs = lhs; // LHS data
  this.rhs = rhs; // RHS data
  this.delta = rhs - lhs; // data range, may be negative
  this.range = Math.abs(this.delta); // Absolute data range
  this.lhsFrame = lhsFrame; // pixel coordinate of LHS side
  this.rhsFrame = rhsFrame; // pixel coordinate of RHS side
  this.width = rhsFrame - lhsFrame; // Width of axis in pixels
  this.lhsBox = lhsBox; // LHS pixel coordinate for possible label extent
  this.rhsBox = rhsBox; // RHS pixel coordinate for possible label extent
  this.fsMax = fsMax; // Maximum allowed font size
  this.fsMin = 5; // Minimum allowed font size
  this.rhoTgt = rhoTgt / pInfo.ppi; // Target tick density in ticks/pixel

  this.Q = [1, 5, 2, 2.5, 4, 3]; // Nice step sizes rank ordered
  this.formats = [0, 1, 2]; // Formats to try in mkLabel

  this.tick = []; // Tick values
  this.pos = []; // Tick positions in pixels
  this.label = []; // Labels
  this.labelPos = []; // Label positions in pixels

  this.yform = function(x, lhs, rhs) { // Transform x to pixels given lhs/rhs data limits
      return Math.round((x - lhs) / (rhs - lhs) * this.width) + this.lhsFrame;
    }

  this.round = function(x) { // round away small floating point issues
      return Math.round(x * 10000) / 10000;
    }

  this.firstTick = function(delta) { // data value of first tick
      return this.round(Math.floor(this.lhs / delta) * delta);
    }

  this.lastTick = function(delta) { // data value of last tick
      return this.round(Math.ceil(this.rhs / delta) * delta);
    }

  this.stride = function(x, delta) { // x offset by delta
      return this.round(x + delta);
    }

  this.qZero = function(x, delta) { // For a tick at x and a step size of d, is zero a tick
      return this.round(Math.round(-x / delta) * delta + x) == 0;
    }

  this.score = function(s, c, d, l) { return 0.2 * s + 0.25 * c + 0.5 * d + 0.05 * l; }
  this.simplicity = function(i, j, qZero) { return 1 - i / (this.Q.length - 1) - (j+1) + qZero; }
  this.density = function(lStep) {
      var ratio = this.range / lStep / Math.abs(this.width) / this.rhoTgt;
      return 2 - Math.max(ratio, 1/ratio);
    } // density
  this.coverage = function(lhs, rhs) {
      var d0 = this.lhs - lhs,
          d1 = this.rhs - rhs,
          d2 = 0.1 * this.range;
      return 1 - (d0 * d0 + d1 * d1) / (d2 * d2) / 2;
    } // coverage

  this.overlapScore = function(dist, fs) {
      var threshold = 0.5 * fs; // 1.5 is not enough
      return (dist >= threshold) ? 1 : (dist > 0) ? (2 - threshold / dist) : -Infinity;
    } // overlapScore

  this.mkTicks = function() { // get tick positions, label positions and text
      var best = {score: -Infinity, pos: [], label: [], labelPos: []},
          nQ = this.Q.length,
          stepNorm = Math.pow(10, Math.round(Math.log(this.range) / Math.LN10) - 1),
          offsetIndices = [], 
          q,
          q0,
          denom,
          lStep,
          qSeen = {},
          key, 
          simp, 
          dens, 
          cov, 
          lhs, 
          rhs, 
          leg, 
          score, 
          i, 
          j, 
          k;

      for (j = 0; j < 10; ++j) {
        offsetIndices.push(j);
        for (i = 0; i < nQ; ++i) {
          q = this.Q[i];
          denom = Math.pow(10, Math.floor(Math.log((j+1) * q) / Math.LN10));
          lStep = (j+1) * q / denom * stepNorm;
          for (k = 0; k <= j; ++k) {
            q0 = q * q * offsetIndices[k] / denom;
            key = lStep + ":" + q0;
            if (key in qSeen) { // Already seen this combination
              continue;
            }
            qSeen[key] = 0;
            simp = this.simplicity(i, j, this.qZero(q0, lStep));
            if (this.score(simp, 1, 1, 1) <= best.score) {
              break;
            }
            dens = this.density(lStep);
            if (this.score(simp, 1, dens, 1) <= best.score) {
              break;
            }
            lhs = this.firstTick(lStep);
            rhs = this.lastTick(lStep);
            cov = this.coverage(lhs, rhs);
            if (this.score(simp, cov, dens, 1) <= best.score) {
              break;
            }
            leg = this.legibility(lhs, rhs, lStep);
            score = this.score(simp, cov, dens, leg.score);
            if (score <= best.score) {
              break;
            }
            best.score = score;
            best.fs = leg.fs;
            best.label = leg.label;
            best.pos = leg.pos;
            best.labelPos = leg.labelPos;
            best.widths = leg.width;
            best.lhs = lhs;
            best.rhs = rhs;
          } // for k
        } // for i
      } // for j

      best.delta = best.rhs - best.lhs;
      best.lhsFrame = this.lhsFrame;
      best.width = this.rhsFrame - this.lhsFrame;
      best.xform = function(x) { // Transform x to pixels using this data limits
          return Math.round((x - this.lhs) / this.delta * this.width) + this.lhsFrame;
        } // xform
      best.iform = function(x) { // Transfer x pixels to data units
          return Math.round((x - this.lhsFrame) / this.width * this.delta) + this.lhs;
        } // iform
      return best;
    } // mkTicks

  this.legibility = function(lhs, rhs, delta) { // How legible will the labels be?
      var best = {score: -Infinity, fs: this.fsMax, label: [], labelPos: [], width: []},
          ctx = pInfo.ctx,
          fontSuffix = pInfo.font,
          qZeroExtended = false,
          nTicks,
          ticks = [],
          pos = [],
          width,
          labelPos,
          labels,
          qLabels,
          qDuplicate,
          qSkip = false,
          netScore,
          iFormat,
          lInfo,
          score,
          fs,
          fsScore,
          fsMax = this.fsMax,
          fsMin = this.fsMin,
          distance,
          overlap,
          aLHS = this.qTight ? this.lhs : lhs,
          aRHS = this.qTight ? this.rhs : rhs,
          i = Math.min(lhs, rhs),
          e = Math.max(lhs, rhs),
          j, je;

      for (;i <= e; i = this.stride(i, delta)) { // Walk through ticks
        ticks.push(i);
        pos.push(this.yform(i, aLHS, aRHS));
        qZeroExtended |= i === 0;
      } // for i

      nTicks = ticks.length;
      best.tick = ticks;
      best.pos = pos;

      for (j = 0, je = this.formats.length; j < je; ++j) { // Walk through formats
        iFormat = this.formats[j];
        labels = [];
        qLabels = {};
        qDuplicate = false;
        netScore = 0;
        for (i = 0; i < nTicks; ++i) {
          lInfo = this.mkLabel(ticks[i], iFormat);
          labels.push(lInfo.label);
          netScore += lInfo.score;
          qSkip |= lInfo.qSkip;
          qDuplicate |= lInfo.label in qLabels;
          if (qDuplicate) { // No need to do anything else
            break;
          }
          qLabels[lInfo.label] = 0;
        } // for i

        if (qSkip && qDuplicate) { // rest of formats will cause duplicates
          break;
        }

        netScore *= 0.9 / Math.max(1, nTicks);
        netScore += 0.1 * qZeroExtended;
        score = (netScore + 3) / 4;
        if (score <= best.score) {
           continue;
        }
        for (fs = fsMax; fs >= fsMin; --fs) { // Walk through font sizes
          fsScore = (fs - fsMin) / Math.max(1, fsMax - fsMin);
          score = (netScore + fsScore + 2) / 4;
          if (score <= best.score) {
            break;
          }
          ctx.font = fs + fontSuffix;
          overlap = Infinity;
          width = [];
          labelPos = [];
          for (i = 0; i < nTicks; ++i) { // Check overlap
            width.push(ctx.measureText(labels[i]).width);
            labelPos.push(pos[i]);
            if (i === 0) { // first tick
              distance = (this.rhsFrame > this.lhsFrame ? 1 : -1) *
                         Math.min(Math.round(Math.abs(labelPos[i]-this.lhsBox)-width[i]/2),0);
              labelPos[i] -= distance;
            }
            if (i === (nTicks - 1)) { // last tick
              distance = (this.rhsFrame > this.lhsFrame ? 1 : -1) *
                         Math.min(Math.round(Math.abs(labelPos[i]-this.rhsBox)-width[i]/2),0);
              labelPos[i] += distance;
            }
            if (i > 0) { // not first tick
              distance = Math.round(Math.abs(labelPos[i-1]-labelPos[i]) -
                                     (width[i-1] + width[i]) / 2);
              overlap = Math.min(overlap, this.overlapScore(distance, fs));
            }
          } // for i

          score = (netScore + fsScore + overlap + 1) / 4;
          if (score <= best.score) {
            continue;
          }
          best.score = score;
          best.fs = fs;
          best.label = labels;
          best.labelPos = labelPos;
          best.width = width;
        } // for fs
      } // for j
      return best;
    } // legibility

  this.mkLabel = function(x, iFormat) {
      var ax = Math.abs(x);

      if (x === 0) { // Zero is the same in all formats
        return {label: '0', score: 1, qSkip: false};
      }

      switch (iFormat) {
        case 0: return {label: x.toPrecision(), score: (ax >= 1e-4) && (ax < 1e6), qSkip: false};
        case 1: return {label: x.toExponential(), score: 0.25, qSkip: false};
        case 2:
          if ((ax >= 1e3) && (ax < 1e6)) {
            return {label: (x * 1e-3).toFixed(0) + "K", score: 0.75, qSkip: true};
          }
          if ((ax >= 1e6) && (ax < 1e9)) {
            return {label: (x * 1e-6).toFixed(0) + "M", score: 0.75, qSkip: true};
          }
          if ((ax >= 1e9) && (ax < 1e12)) {
            return {label: (x * 1e-9).toFixed(0) + "G", score: 0.75, qSkip: true};
          }
      }
      return {label: x, score: 0, qSkip: false};
    } // mkLabel
}

function rotText(ctx, str, x, y) {
  ctx.save();
  ctx.setTransform(0,-1,1,0,x,y); // Rotate by -90 degrees and translate to x and y
  // ctx.translate(x, y);
  // ctx.rotate(-Math.PI / 2);
  ctx.fillText(str, 0, 0);
  ctx.restore();
} // rotText

function myMouse(e) {
  var taxis = pInfo.taxis,
      pop = pInfo.pop,
      cX = e.clientX || e.touchs[0].clientX,
      t = taxis.iform(cX - pInfo.left),
      y,
      tt = pInfo.t,
      yy = pInfo.y,
      n = tt.length - 1,
      iStart = 0,
      iStop = n,
      iMid,
      dist0,
      date;

  while (iStart < iStop) {
    iMid = Math.floor((iStart + iStop) / 2); // Mid point
    if (tt[iMid] < t) {
      iStart = iMid + 1;
    } else {
      iStop = iMid - 1;
    }
  } // while

  t = tt[iStart];
  dist0 = Math.abs(tt[iStart] - t);
  iStart = iStart -
           (Math.abs(tt[iStart - (iStart > 0)] - t) < dist0) +
           (Math.abs(tt[iStart + (iStart < n)] - t) < dist0);

  if ('timeout' in pInfo) {
    if (pInfo.iPrev == iStart) { // no change;
      return;
    }
    clearTimeout(pInfo.timeout);
    delete(pInfo.timeout);
  }

  if ('left' in pInfo.dot) {
    pop.clearRect(pInfo.dot.left, pInfo.dot.top, pInfo.dot.size, pInfo.dot.size);
  }

  t = tt[iStart];
  y = yy[iStart];

  pInfo.iPrev = iStart;
  pInfo.dot.x = taxis.xform(t);
  pInfo.dot.y = pInfo.yaxis.xform(y);
  pInfo.dot.left = pInfo.dot.x - pInfo.dot.r - 1;
  pInfo.dot.top = pInfo.dot.y - pInfo.dot.r - 1;

  pop.beginPath();
  pop.arc(pInfo.dot.x, pInfo.dot.y, pInfo.dot.r, 0, 2 * Math.PI);
  pop.fill();
  pop.stroke();

  date = new Date(t * 1000);
  pop.clearRect(pInfo.txt.left, pInfo.txt.top, pInfo.txt.width, pInfo.txt.height);
  pop.fillText(y + "(" + pInfo.yunits + ") " + date.toLocaleString(),
               pInfo.txt.x, pInfo.txt.y);

  pInfo.timeout = setTimeout(function() {
      var pop = pInfo.pop;
      clearTimeout(pInfo.timeout);
      delete(pInfo.timeout);
      pop.clearRect(pInfo.dot.left, pInfo.dot.top, pInfo.dot.size, pInfo.dot.size);
      pop.clearRect(pInfo.txt.left, pInfo.txt.top, pInfo.txt.width, pInfo.txt.height);
      pop.fillText(pInfo.genOn, pInfo.txt.x, pInfo.txt.y);
    }, 10000); // 10 seconds
} // myMouse

function initPlt() {
  var div = mkDiv(), // Where to put everything
      now = new Date,
      canvas0 = document.createElement("canvas"),
      canvas1 = document.createElement("canvas"),
      ctx = canvas0.getContext("2d"),
      dpr = window.devicePixelRatio || 1,        // Device pixel ratio 
      bsr;

  canvas0.id = "pltCanvas";
  canvas1.id = "popCanvas";

  canvas1.addEventListener("mousemove", function() {myMouse(event);});
  canvas1.addEventListener("touchmove", function() {myMouse(event);});

  div.appendChild(canvas0);
  div.appendChild(canvas1);

  pInfo.ctx = ctx;
  pInfo.pop = canvas1.getContext("2d");
  pInfo.genOn = "Drawn " + (now.getMonth() + 1) + "/" + now.getDate() + "/" +
                format2(now.getFullYear() % 100) + " " +
                now.getHours() + ":" + format2(now.getMinutes()),
  pInfo.txt = {x: 0,
               y: 0,
               top: 0,
               left: 0,
               width: 0,
               height: 0};

  pInfo.font = "px Arial";

  bsr = ctx.webkitBackingStorePixelRatio ||  // backing store pixel ratio
        ctx.mozBackingStorePixelRatio ||
        ctx.msBackingStorePixelRatio ||
        ctx.oBackingStorePixelRatio ||
        ctx.backingStorePixelRatio || 1;
  pInfo.ppi = 96 * dpr / bsr;                      // By default web is at 96dpi,

  pInfo.dot = {r: Math.ceil(pInfo.ppi * 0.1)};
  pInfo.dot.size = 2 * pInfo.dot.r + 2;
} // initPlt

function noCanvas() { // Browser does not support a canvas, so fall back to image
  var a = document.createElement("img");
  a.src = "/kayaking2/cgi/png?" + buildQueryString(["h", "t", "sdate", "edate"]);
  a.width = 800;
  a.height = 600;
  mkDiv().appendChild(a);
}

function addData(d) {
  var names = ["title", "type", 
               "low", "high", "lowOptimal", "highOptimal",
               "t", "tMin", "tMax", "tlabel", "tunits",
               "y",  "yMin",  "yMax",  "ylabel",  "yunits", 
               "y1", "y1Min", "y1Max", "y1label", "y1units",
               "y2", "y2Min", "y2Max", "y2label", "y2units",
               "y3", "y3Min", "y3Max", "y3label", "y3units",
               "y4", "y4Min", "y4Max", "y4label", "y4units"],
      i, 
      e;

  "error" in d && errmsg(d.error);

  for (i = 0, e = names.length; i < e; ++i) {
    names[i] in d && (pInfo[names[i]] = d[names[i]]);
  }
} // addData

function initTblPlt(d, mode) {
  var names = ["t", "y", "y1", "y2", "y3", "y4"],
      i, e;

  for (i = 0, e = names.length; i < e; ++i) {
    (names[i] in d) && decodeData(d, names[i]);
  }

  addData(d);

  document.title = pInfo.title;

  pInfo.qCanvas = qCanvas();
  pInfo.mode = mode;
  updateForm(); // Update the start/stop dates
} // initTblPlt 

function decodeData(d, name) {
  var enc = name + "Enc",
      data, a, b, i, e;

  if (name in d && enc in d) {
    data = d[name];
    a = d[enc];
    if (a == "Step") {
      a = d[name + "Step"];
      b = d[name + "Min"];
      for (i = 0, e = data.length; i < e; ++i) {
        data[i] = (data[i] * a) + b;
      }
    } else if (a == "Min") {
      b = d[name + "Min"];
      for (i = 0, e = data.length; i < e; ++i) {
        data[i] = data[i] + b;
      }
    } else if (a == "Map") {
      b = d[name + "Map"];
      for (i = 0, e = data.length; i < e; ++i) {
        data[i] = b[data[i]];
      }
    } else {
      console.log("Unrecognized encoding '" + a + "'");
    }
    delete d[enc];
  }
} // decodeData

function updateForm(frm) {
  var x = (frm ? frm : document.getElementById("frm")).elements,
      d0 = new Date(pInfo.tMin * 1000),
      d1 = new Date(pInfo.tMax * 1000);

  pInfo.sdate = d0;
  pInfo.edate = d1;

  setDate(x.sdate, d0).focus();
  setDate(x.edate, d1);

  // Set no-canvas variable, if canvas is not supported by this browser
 
  x.nc.value = pInfo.qCanvas ? "0" : "1";

  // Since we are running, we know javascript is enabled. So modify the submit buttons
  // to callbacks
  x.o[1].type = "button";
  x.o[1].onclick = function() {frmClick(event);}

  if (pInfo.qCanvas) {
    x.o[0].type = "button";
    x.o[0].onclick = function() {frmClick(event);}
  }
} // updateForm

function setDate(e, date) { // Set the date, depending on the form input type
  if (e.type === "date") { // browser supports date input type
    date.setHours(0);  // Set to noon
    date.setMinutes(0); 
    date.setSeconds(0); 
    date.setMilliseconds(0);
    e.valueAsDate = date;
  } else {
    e.value = (date.getMonth() + 1) + "/" + date.getDate() + "/" + date.getFullYear();
  }
  return e;
} // setDate

function getDate(e) { // Get date from input e
  var t;

  if (e.type === "date") { // browser supports input type date
    t = new Date(e.value + " 00:00:00"); // Force to midnight local
    return t;
  }
  t = Date.parse(e.value); // Turn into milliseconds
  if (t) {
    return new Date(t);
  }

  t = "Invalid date string, '" + e.value + "'";
  alert(t);
  throw(t);
} // getDate

function getDates(f) { // Get sdate and edate from form f, and return as an object
  var d0 = getDate(f.sdate),
      d1 = getDate(f.edate); 
  return {sdate: d0 < d1 ? d0 : d1,
          edate: d0 < d1 ? d1 : d0};
} // getDates

function frmClick(e) {
  var tgt = e.target,
      frm = tgt.parentNode,
      dates = getDates(frm),
      qChanged = (pInfo.type !== frm.t.value) ||
                 ((dates.sdate - pInfo.sdate) !== 0) || 
                 ((dates.edate - pInfo.edate) !== 0),
      fmtDate = function(d) {return d.getFullYear() + "-" + (d.getMonth()+1) + "-" + d.getDate()},
      url,
      xhr,
      cleanup = function() {
          "timeout" in pInfo && clearTimeout(pInfo.timeout);
          document.body.removeChild(document.getElementById("addon"));
          window.onresize = null;
        };

  if (!qChanged) {
    url = tgt.value;
    if (url != pInfo.mode) {
      pInfo.mode = url;
      cleanup();
      return url === "Plot" ? mkPlt() : mkTbl();
    }
    return;
  }

  // Fetch new data

  url = "?h=" + frm.h.value + 
        "&t=" + frm.t.value +
        "&o=d&sdate=" + fmtDate(dates.sdate) + 
        "&edate=" + fmtDate(dates.edate);
  xhr = new XMLHttpRequest();
  xhr.open("GET", url);
  xhr.onreadystatechange = function(e) {
      if (this.readyState == 4) { // finished
        if (this.status == 200) { // good status
          cleanup();
          pInfo = {};
          eval((tgt.value === "Plot" ? "plt(" : "tbl(") + this.responseText + ");");
        } else {
          alert("Bad status getting '" + url + "'");
        }
      }
    };
  xhr.send();
} // frmClick

function qCanvas() {
  var canvas = document.createElement("canvas");
  return canvas && canvas.getContext("2d");
} // qCanvas

function t2str(t, qYear) {
  var date = new Date(t * 1000),
      mon = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"],
      hour = date.getHours(),
      min = date.getMinutes(),
      str = date.getDate() + "-" + mon[date.getMonth()];

  qYear && (str += "-" + date.getFullYear());
  return str + 
         " " + (hour < 10 ? "0" + hour : hour) +
         ":" + (min < 10 ? "0" + min : min); 
} // t2str

function format2(x) {
  return x < 10 ? ("0" + x) : x;
} // format2

function buildQueryString(keys) {
  var parts = location.search.substr(1).split("&"),
      params = {},
      str = "",
      i, 
      e,
      items;

  for (i = 0, e = parts.length; i < e; ++i) {
    items = parts[i].split("=", 2);
    params[items[0]] = items[1];
  }

  for (i = 0, e = keys.length; i < e; ++i) {
    if ((keys[i] in params) && (params[keys[i]] !== "")) {
      if (str !== "") {
        str += "&amp;"; 
      }
      str += keys[i] + "=" + params[keys[i]];
    }
  }

  return str;
}

function mkDiv() {
  var a = document.createElement("div");
  a.id = "addon";
  document.body.appendChild(a);
  return a;
}

function rmDiv() {
  return document.body.removeChild(document.getElementById("addon"));
}

function myAlert(msg) {
  alert(msg);
  throw msg;
}
