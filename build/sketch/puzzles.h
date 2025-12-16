#line 1 "/Users/nickcuskey/xmasLockboxes/ChristmasLockbox/puzzles.h"
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Christmas Lockbox</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: sans-serif; text-align: center; background: #e0e0e0; padding-top: 50px; }
.btn { display: block; width: 80%; max-width: 400px; margin: 20px auto; padding: 25px; font-size: 24px; border-radius: 15px; text-decoration: none; color: white; box-shadow: 0 4px 6px rgba(0,0,0,0.2); transition: transform 0.1s; }
.btn:active { transform: scale(0.98); }
.sam { background: linear-gradient(135deg, #d32f2f, #b71c1c); }
.kris { background: linear-gradient(135deg, #388e3c, #1b5e20); }
.jacob { background: linear-gradient(135deg, #1976d2, #0d47a1); }
h1 { color: #333; }
p { font-size: 18px; color: #555; }
</style>
</head><body>
<h1>🎄 North Pole Access 🎄</h1>
<p>Select your user profile:</p>
<a href="/sam" class="btn sam">🎅 Sam (Santa's Helper)</a>
<a href="/kristine" class="btn kris">📡 Kristine (Elf Uplink)</a>
<a href="/jacob" class="btn jacob">💻 Jacob (SysAdmin)</a>
</body></html>
)rawliteral";

// ===================================================================================
// PUZZLE 1: SAM (Santa's Helper - 3 Stages)
// ===================================================================================
const char puzzle1_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Santa's Helper</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { background: #d32f2f; font-family: 'Comic Sans MS', sans-serif; text-align: center; color: white; margin: 0; padding: 20px; }
.grid { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 400px; margin: 20px auto; }
.card { background: white; aspect-ratio: 1; border-radius: 12px; font-size: 40px; display: flex; align-items: center; justify-content: center; cursor: pointer; user-select: none; box-shadow: 0 4px 0 #999; }
.hidden span { display: none; }
.card.hidden { background: #ffcdd2; background-image: url('data:image/svg+xml;utf8,<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100"><text y="50%" x="50%" dy=".3em" text-anchor="middle" font-size="50">❓</text></svg>'); }
.matched { background: #a5d6a7; opacity: 0.6; pointer-events: none; }
h1 { margin-bottom: 5px; text-shadow: 2px 2px 0 #333; }
h2 { margin-top: 0; font-size: 18px; color: #ffebd3; }
.stage-indicator { display: flex; justify-content: center; gap: 5px; margin-bottom: 15px; }
.dot { width: 15px; height: 15px; background: rgba(255,255,255,0.3); border-radius: 50%; }
.dot.active { background: #fff; box-shadow: 0 0 10px white; }
#message { min-height: 30px; font-weight: bold; font-size: 20px; color: #ffeb3b; }
</style>
</head><body>
<div id="ui">
  <h1>🎅 Santa's Helper</h1>
  <h2 id="subtitle">Loading...</h2>
  <div class="stage-indicator">
    <div id="d1" class="dot active"></div><div id="d2" class="dot"></div><div id="d3" class="dot"></div>
  </div>
  <div id="message"></div>
  <div class="grid" id="game"></div>
</div>

<script>
let stage = 1;
const stages = [
  { name: "The Stable", task: "Harness the Reindeer!", icons: ['🦌','🦌','🥕','🥕','🔔','🔔','🛷','🛷','🧣','🧣','🎅','🎅'] },
  { name: "The Workshop", task: "Pack the Toys!", icons: ['🎁','🎁','🧸','🧸','🚂','🚂','🥁','🥁','🎮','🎮','🎨','🎨'] },
  { name: "The Kitchen", task: "Fuel for Santa!", icons: ['🍪','🍪','🥛','🥛','🍬','🍬','🧁','🧁','🍩','🍩','🍭','🍭'] }
];

let cards = [], flipped = [], matched = 0;

function shuffle(a) { for(let i=a.length-1;i>0;i--){const j=Math.floor(Math.random()*(i+1));[a[i],a[j]]=[a[j],a[i]];} return a; }

function loadStage() {
  const s = stages[stage-1];
  document.getElementById('subtitle').innerText = "Stage " + stage + ": " + s.name;
  document.getElementById('message').innerText = s.task;
  
  // Update dots
  document.getElementById('d1').className = stage >= 1 ? 'dot active' : 'dot';
  document.getElementById('d2').className = stage >= 2 ? 'dot active' : 'dot';
  document.getElementById('d3').className = stage >= 3 ? 'dot active' : 'dot';

  matched = 0;
  flipped = [];
  const game = document.getElementById('game');
  game.innerHTML = '';
  
  shuffle([...s.icons]).forEach((txt) => {
    let d = document.createElement('div');
    d.className = 'card hidden';
    d.innerHTML = '<span>'+txt+'</span>';
    d.onclick = () => flip(d, txt);
    game.appendChild(d);
  });
}

function flip(el, txt) {
  if(flipped.length >= 2 || el.className.includes('matched') || !el.className.includes('hidden')) return;
  el.className = 'card';
  flipped.push({el, txt});
  if(flipped.length === 2) check();
}

function check() {
  const [a, b] = flipped;
  if(a.txt === b.txt) {
    a.el.classList.add('matched'); b.el.classList.add('matched');
    matched++;
    flipped = [];
    if(matched === 6) winStage();
  } else {
    setTimeout(() => {
      a.el.className = 'card hidden'; b.el.className = 'card hidden';
      flipped = [];
    }, 800);
  }
}

function winStage() {
  document.getElementById('message').innerText = "Good Job! 🎉";
  setTimeout(() => {
    if(stage < 3) {
      stage++;
      loadStage();
    } else {
      document.body.innerHTML = "<div style='margin-top:50%;'><h1>🎉 COMPLETE! 🎉</h1><p>Sleigh is ready!</p></div>";
      fetch('/solve?puzzle=1').then(() => setTimeout(() => window.location.href='/', 3000));
    }
  }, 1500);
}

loadStage();
</script></body></html>
)rawliteral";

// ===================================================================================
// PUZZLE 2: KRISTINE (Elf Comm-Link - 3 Stages)
// ===================================================================================
const char puzzle2_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>Elf Comm-Link</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { background: #1b5e20; font-family: monospace; text-align: center; color: #a5d6a7; display: flex; flex-direction: column; align-items: center; min-height: 100vh; margin: 0; }
h1 { color: #fff; margin-bottom: 5px; }
.status { color: #fe5555; animation: blink 1s infinite; font-weight: bold; margin-bottom: 20px;}
@keyframes blink { 50% { opacity: 0; } }
.board { display: grid; gap: 4px; margin-bottom: 20px; }
.row { display: grid; grid-template-columns: repeat(5, 1fr); gap: 4px; }
.tile { width: 45px; height: 45px; background: #2e7d32; border: 2px solid #4caf50; font-size: 24px; font-weight: bold; display: flex; align-items: center; justify-content: center; color: white; }
.tile.correct { background: #43a047; border-color: #fff; } 
.tile.present { background: #fdd835; color: #000; border-color: #fff; } 
.tile.absent { background: #1b5e20; opacity: 0.5; }
.keyboard { display: flex; flex-direction: column; gap: 5px; width: 100%; max-width: 500px; }
.key-row { display: flex; justify-content: center; gap: 3px; }
.key { padding: 10px; background: #66bb6a; color: #000; border-radius: 4px; cursor: pointer; font-weight: bold; user-select: none; }
.key.wide { padding: 10px 20px; }
#message { height: 20px; color: #fff; margin-bottom: 10px; }
</style>
</head><body>
<h1>📡 Elf Uplink</h1>
<div id="status" class="status">OFFLINE - RECALIBRATING...</div>
<div id="message">Target: FREQUENCY_CAL</div>

<div class="board" id="board"></div>

<div class="keyboard" id="keyboard"></div>

<script>
let stage = 1;
const stages = [
  { word: "FROST", hint: "Stage 1: Calibration (Password: The cold stuff)" },
  { word: "JINGL", hint: "Stage 2: Decryption (Password: Sound of bells)" },
  { word: "MERRY", hint: "Stage 3: Authorization (Password: Have a ____ Christmas)" }
];

let currentRow = 0;
let currentGuess = "";
let gameOver = false;

function initStage() {
  document.getElementById('status').innerText = "Stage " + stage + "/3: LINKING...";
  document.getElementById('message').innerText = stages[stage-1].hint;
  document.getElementById('board').innerHTML = '';
  currentRow = 0;
  currentGuess = "";
  gameOver = false;
  
  for(let i=0; i<6; i++) {
    let r = document.createElement('div');
    r.className = 'row';
    for(let j=0; j<5; j++) {
      let t = document.createElement('div');
      t.className = 'tile';
      t.id = 'r'+i+'c'+j;
      r.appendChild(t);
    }
    document.getElementById('board').appendChild(r);
  }
  createKeyboard();
}

function createKeyboard() {
  const keys = ["QWERTYUIOP","ASDFGHJKL","ZXCVBNM"];
  const kb = document.getElementById('keyboard');
  kb.innerHTML = '';
  keys.forEach((row, i) => {
    let d = document.createElement('div');
    d.className = 'key-row';
    if(i===2) {
       let enter = document.createElement('div'); enter.className='key wide'; enter.innerText='ENT'; enter.onclick=submit; d.appendChild(enter);
    }
    row.split('').forEach(k => {
      let key = document.createElement('div');
      key.className = 'key';
      key.innerText = k;
      key.onclick = () => type(k);
      d.appendChild(key);
    });
    if(i===2) {
       let del = document.createElement('div'); del.className='key wide'; del.innerText='⌫'; del.onclick=backspace; d.appendChild(del);
    }
    kb.appendChild(d);
  });
}

function type(char) {
  if(gameOver || currentGuess.length >= 5) return;
  currentGuess += char;
  updateGrid();
}

function backspace() {
  if(gameOver || currentGuess.length === 0) return;
  currentGuess = currentGuess.slice(0,-1);
  updateGrid();
}

function updateGrid() {
  for(let i=0; i<5; i++) {
    document.getElementById('r'+currentRow+'c'+i).innerText = currentGuess[i] || "";
  }
}

function submit() {
  if(gameOver || currentGuess.length !== 5) return;
  
  const secret = stages[stage-1].word;
  const guess = currentGuess;
  let s = secret.split('');
  let g = guess.split('');
  let colors = new Array(5).fill('absent');

  // Green pass
  for(let i=0; i<5; i++) {
    if(g[i] === s[i]) { colors[i] = 'correct'; s[i] = null; g[i] = null; }
  }
  // Yellow pass
  for(let i=0; i<5; i++) {
    if(g[i] && s.includes(g[i])) { 
      colors[i] = 'present'; s[s.indexOf(g[i])] = null; 
    }
  }

  // Paint
  for(let i=0; i<5; i++) {
    const tile = document.getElementById('r'+currentRow+'c'+i);
    tile.className = 'tile ' + colors[i];
    tile.style.transitionDelay = (i*100)+'ms';
  }

  if(guess === secret) {
    gameOver = true;
    document.getElementById('status').innerText = "ACCESS GRANTED";
    document.getElementById('status').style.color = "#4caf50";
    setTimeout(() => {
        if(stage < 3) { stage++; initStage(); }
        else {
             alert("UPLINK RESTORED! 📡");
             fetch('/solve?puzzle=2').then(() => window.location.href='/');
        }
    }, 1500);
  } else {
    currentRow++;
    currentGuess = "";
    if(currentRow === 6) {
        alert("Try again!");
        currentRow = 0; currentGuess = ""; updateGrid(); 
        document.getElementById('board').innerHTML = ''; // lazy reset
        initStage();
    }
  }
}
initStage();
</script></body></html>
)rawliteral";

// ===================================================================================
// PUZZLE 3: JACOB (SysAdmin - 3 Stages)
// ===================================================================================
const char puzzle3_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><title>NP-SYSADMIN</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { background: #000; color: #00ff00; font-family: 'Courier New', monospace; padding: 10px; margin: 0; font-size: 14px; }
#console { white-space: pre-wrap; margin-bottom: 50px; }
.input-line { display: flex; position: fixed; bottom: 0; left: 0; width: 100%; background: #111; padding: 10px; border-top: 1px solid #333; }
input { background: transparent; border: none; color: #00ff00; font-family: inherit; font-size: 16px; flex-grow: 1; outline: none; }
.prompt { margin-right: 10px; color: #00ff00; }
.error { color: #ff0000; }
.sys { color: #00ccff; }
</style>
</head><body>
<div id="console"></div>
<div class="input-line">
  <span class="prompt" id="ps1">guest@npole:~$</span>
  <input type="text" id="cmd" autofocus autocomplete="off" autocorrect="off" autocapitalize="off" spellcheck="false">
</div>
<script>
let stage = 1;
const out = document.getElementById('console');
const inp = document.getElementById('cmd');
const ps1 = document.getElementById('ps1');

function print(txt, type='') { out.innerHTML += `<span class="${type}">${txt}</span>\n`; window.scrollTo(0,document.body.scrollHeight); }

inp.addEventListener('keydown', (e) => {
  if (e.key === 'Enter') {
    const val = inp.value;
    print(ps1.innerText + " " + val);
    inp.value = '';
    process(val.trim());
  }
});

function boot() {
  print("NORTH POLE MAINFRAME v9.0.1", "sys");
  print("Scanning systems... CRITICAL ERROR FOUND.", "error");
  print("Database locked by KRAMPUS ROOTKIT.", "error");
  print("\nInitializing recovery shell...");
  print("STAGE 1: SHELL ACCESS. Find the password hint in 'readme.txt'.");
  print("\nType 'help' for commands.");
}

function process(cmd) {
  const args = cmd.toLowerCase().split(' ');
  const c = args[0];

  if(c === 'help') {
    print("Commands: ls, cat [file], login [pass], answer [val], fix [disk], help");
    return;
  }

  // STAGE 1: SHELL ACCESS
  if(stage === 1) {
    if(c === 'ls') print("readme.txt  sys.log  login.exe");
    else if(c === 'cat' && args[1] === 'readme.txt') print("Hint: I contain cities, mountains, and oceans, but no land. What am I? (One word)");
    else if(c === 'login') {
       if(args[1] === 'map') {
           print("ACCESS GRANTED. Welcome, Admin.", "sys");
           print("\nSTAGE 2: FIREWALL BREACH.", "sys");
           print("Calculated port 443 offset required.");
           print("Solve: (0x10 + 16) * 2 = ?");
           print("Type 'answer [number]'");
           stage = 2;
           ps1.innerText = "admin@npole:~#";
       } else print("Access Denied.", "error");
    }
    else if(c === '') {}
    else print("Command not found or permission denied.", "error");
  } 
  // STAGE 2: FIREWALL
  else if (stage === 2) {
      if(c === 'answer') {
          if(args[1] === '64') {
              print("FIREWALL BYPASSED.", "sys");
              print("\nSTAGE 3: KERNEL RECOVERY.", "sys");
              print("Filesystem corrupted. /dev/sda1 blocks bad.");
              print("Run 'fix /dev/sda1' to repair.");
              stage = 3;
          } else print("Incorrect checksum.", "error");
      }
      else print("Firewall active. Command blocked.", "error");
  }
  // STAGE 3: RECOVERY
  else if (stage === 3) {
      if(c === 'fix' && args[1] === '/dev/sda1') {
          print("Repairing sectors... [||||||||||] 100%");
          print("Krampus Rootkit REMOVED.", "sys");
          print("System Normal. Naughty/Nice DB unlocked.");
          print("\nREBOOTING...");
          setTimeout(() => {
              fetch('/solve?puzzle=3').then(() => window.location.href='/');
          }, 3000);
      } else if (c === 'ls') print("/dev/sda1  /dev/sda2");
      else print("Unknown command.", "error");
  }
}

boot();
</script></body></html>
)rawliteral";
