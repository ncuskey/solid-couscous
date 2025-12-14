/**
 * Modular Game Engines
 */

/**
 * MEMORY GAME
 * Config: { icons: [], title: "", task: "" }
 */
class MemoryGame {
    constructor(containerId, config, onComplete) {
        this.container = document.getElementById(containerId);
        this.config = config;
        this.onComplete = onComplete;
        this.flippedCards = [];
        this.matchedCount = 0;
        this.totalPairs = config.icons.length / 2;

        this.render();
    }

    render() {
        this.container.innerHTML = `
            <div class="game-header">
                <h2>${this.config.title}</h2>
                <p>${this.config.task}</p>
            </div>
            <div class="memory-grid"></div>
        `;

        const grid = this.container.querySelector('.memory-grid');
        this.shuffle(this.config.icons).forEach(icon => {
            const card = document.createElement('div');
            card.className = 'card-container';
            card.innerHTML = `
                <div class="card-face card-front"></div>
                <div class="card-face card-back">${icon}</div>
            `;
            card.onclick = () => this.flip(card, icon);
            grid.appendChild(card);
        });
    }

    shuffle(a) {
        const b = [...a];
        for (let i = b.length - 1; i > 0; i--) {
            const j = Math.floor(Math.random() * (i + 1));
            [b[i], b[j]] = [b[j], b[i]];
        }
        return b;
    }

    flip(el, txt) {
        if (this.flippedCards.length >= 2 || el.classList.contains('flipped') || el.classList.contains('matched')) return;

        el.classList.add('flipped');
        this.flippedCards.push({ el, txt });

        if (this.flippedCards.length === 2) this.check();
    }

    check() {
        const [a, b] = this.flippedCards;
        if (a.txt === b.txt) {
            this.matchedCount++;
            this.flippedCards = [];
            setTimeout(() => {
                a.el.classList.add('matched');
                b.el.classList.add('matched');
                if (this.matchedCount === this.totalPairs) {
                    setTimeout(this.onComplete, 1000);
                }
            }, 300);
        } else {
            setTimeout(() => {
                a.el.classList.remove('flipped');
                b.el.classList.remove('flipped');
                this.flippedCards = [];
            }, 1000);
        }
    }
}

/**
 * WORDLE GAME
 * Config: { word: "HELLO", hint: "" }
 */
class WordleGame {
    constructor(containerId, config, onComplete) {
        this.container = document.getElementById(containerId);
        this.config = config;
        this.onComplete = onComplete;
        this.currentRow = 0;
        this.currentGuess = "";
        this.gameOver = false;

        this.render();
        this.bindEvents();
    }

    render() {
        this.container.innerHTML = `
            <div class="game-header">
                <h2>${this.config.hint}</h2>
            </div>
            <div class="wordle-board"></div>
            <div class="keyboard"></div>
        `;

        const board = this.container.querySelector('.wordle-board');
        for (let i = 0; i < 6; i++) {
            let row = document.createElement('div');
            row.className = 'wordle-row';
            for (let j = 0; j < 5; j++) {
                let tile = document.createElement('div');
                tile.className = 'wordle-tile';
                tile.id = `r${i}c${j}`;
                row.appendChild(tile);
            }
            board.appendChild(row);
        }

        const kb = this.container.querySelector('.keyboard');
        const keys = ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"];
        keys.forEach((row, i) => {
            let r = document.createElement('div');
            r.className = 'key-row';
            if (i === 2) {
                let ent = document.createElement('div'); ent.className = 'key wide'; ent.innerText = 'ENT';
                ent.onclick = () => this.submit(); r.appendChild(ent);
            }
            row.split('').forEach(k => {
                let key = document.createElement('div'); key.className = 'key'; key.innerText = k;
                key.onclick = () => this.type(k); r.appendChild(key);
            });
            if (i === 2) {
                let del = document.createElement('div'); del.className = 'key wide'; del.innerText = '⌫';
                del.onclick = () => this.backspace(); r.appendChild(del);
            }
            kb.appendChild(r);
        });
    }

    bindEvents() {
        // Simple click binding is handled inline, but we could add global keys here
    }

    type(char) {
        if (this.gameOver || this.currentGuess.length >= 5) return;
        this.currentGuess += char;
        this.updateGrid();
    }

    backspace() {
        if (this.gameOver || this.currentGuess.length === 0) return;
        this.currentGuess = this.currentGuess.slice(0, -1);
        this.updateGrid();
    }

    updateGrid() {
        for (let i = 0; i < 5; i++) {
            const tile = document.getElementById(`r${this.currentRow}c${i}`);
            tile.innerText = this.currentGuess[i] || "";
            if (i === this.currentGuess.length) tile.classList.add('active-input');
            else tile.classList.remove('active-input');
        }
    }

    submit() {
        if (this.currentGuess.length !== 5) return;

        const secret = this.config.word;
        const guess = this.currentGuess;
        let s = secret.split('');
        let g = guess.split('');
        let colors = Array(5).fill('absent');

        // Green pass
        for (let i = 0; i < 5; i++) {
            if (g[i] === s[i]) { colors[i] = 'correct'; s[i] = null; g[i] = null; }
        }
        // Yellow pass
        for (let i = 0; i < 5; i++) {
            if (g[i] && s.includes(g[i])) {
                colors[i] = 'present'; s[s.indexOf(g[i])] = null;
            }
        }

        for (let i = 0; i < 5; i++) {
            const tile = document.getElementById(`r${this.currentRow}c${i}`);
            setTimeout(() => {
                tile.classList.add(colors[i]);
                tile.style.transform = "scale(1.1)";
                setTimeout(() => tile.style.transform = "scale(1)", 200);
            }, i * 100);
        }

        setTimeout(() => {
            if (guess === secret) {
                this.gameOver = true;
                setTimeout(this.onComplete, 1000);
            } else {
                this.currentRow++;
                this.currentGuess = "";
                if (this.currentRow === 6) {
                    alert('Try Again!');
                    this.reset();
                }
            }
        }, 600);
    }

    reset() {
        this.container.innerHTML = '';
        this.render();
        this.currentRow = 0;
        this.currentGuess = "";
        this.gameOver = false;
    }
}

/**
 * TERMINAL GAME
 * Config: { user: "guest", logic: (cmd, print, complete) => void }
 */
class TerminalGame {
    constructor(containerId, config, onComplete) {
        this.container = document.getElementById(containerId);
        this.config = config;
        this.onComplete = onComplete;
        this.render();
    }

    render() {
        this.container.innerHTML = `
            <div class="terminal-container">
                <div class="terminal-output"></div>
                <div class="input-line">
                    <span class="prompt">${this.config.user}@npole:~$</span>
                    <input type="text" class="terminal-input" autofocus autocomplete="off" spellcheck="false">
                </div>
            </div>
        `;

        this.output = this.container.querySelector('.terminal-output');
        this.input = this.container.querySelector('.terminal-input');

        this.input.addEventListener('keydown', (e) => {
            if (e.key === 'Enter') {
                const val = this.input.value;
                this.print(`${this.config.user}@npole:~$ ${val}`);
                this.input.value = '';
                this.config.logic(val.trim(), this.print.bind(this), this.onComplete);
            }
        });
        this.container.addEventListener('click', () => this.input.focus());

        if (g[i] === s[i]) { colors[i] = 'correct'; s[i]
