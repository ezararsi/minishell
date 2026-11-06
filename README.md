# minishell

Eine Unix-Shell in C nach dem Vorbild von bash, entstanden als 42-Projekt in Zusammenarbeit mit einer zweiten Person.
Schwerpunkte sind Prozesse, Pipes, File-Descriptoren und Signale.
Umgesetzt ist der Pflichtteil aus dem Subject, in ca. 1.400 Zeilen.

> **Hinweis:** Ursprünglich 2023/24 an der 42 Vienna entstanden, 2026 überarbeitet und hier neu veröffentlicht – daher stammen Git-Historie und Datei-Header aus 2026.

---

## Bauen & starten

**Linux / WSL (Ubuntu)**

```bash
sudo apt install build-essential libreadline-dev
make
./minishell
```

**Windows**: nur über WSL, da die Shell Linux-Systemaufrufe verwendet:

```powershell
.\minishell.bat
```

**macOS**

```bash
xcode-select --install     # Compiler, einmalig
brew install readline      # GNU readline, einmalig
make
./minishell
```

Make-Ziele: `make`, `make clean`, `make fclean`, `make re`, `make test`.

Die Shell liest auch aus einer Pipe oder Datei, dann ohne Prompt: `echo "ls | wc -l" | ./minishell`.

## Tests

```bash
make test
```

`tests/run_tests.sh` führt 40 Eingaben sowohl in minishell als auch in bash aus und vergleicht Ausgabe und Exit-Code
(Quotes, Variablen, Pipes, Umleitungen, Heredocs, Builtins, Syntaxfehler).
Zusätzlich mit valgrind geprüft: keine verlorenen Speicherblöcke.

---

## Funktionen

- Prompt mit History (Pfeiltasten) über readline
- Externe Programme über `$PATH` (`ls`, `grep`, `cat`, …) mit `fork()` + `execve()`
- Pipes beliebiger Länge: `ls -l | grep .c | wc -l`
- Umleitungen `<`, `>`, `>>` und Heredoc `<<` (`<< 'EOF'` ohne Variablen-Expansion)
- Variablen `$VAR`, `$?`, einfache `'…'` und doppelte `"…"` Quotes
- Builtins: `echo [-n]`, `cd`, `pwd`, `export`, `unset`, `env`, `exit`
- Exit-Codes wie bash: `127` nicht gefunden, `126` nicht ausführbar, `2` Syntaxfehler, `128+n` bei Signal

### Signale

| Taste | am Prompt | während ein Befehl läuft |
|---|---|---|
| Ctrl-C | neue Zeile, `$? = 130` | Befehl wird beendet, `$? = 130` |
| Ctrl-\ | ignoriert | `Quit (core dumped)`, `$? = 131` |
| Ctrl-D | Shell beenden | EOF für das Programm |

---

## Aufbau

```
Eingabe ──► lexer ──► parser ──► heredocs ──► execute
            Tokens    Befehle    vorab lesen  fork / pipe / dup2 / execve
```

| Datei | Aufgabe |
|---|---|
| `main.c` | Hauptschleife: Zeile lesen, verarbeiten, wiederholen |
| `input.c` | Liest eine Zeile: im Terminal mit readline, sonst direkt von stdin |
| `lexer.c` | Zerlegt die Eingabe in Wörter und Operatoren (`\|`, `<`, `>`, …) |
| `parser.c` | Baut eine Liste von Befehlen, prüft die Syntax |
| `expand.c` | Ersetzt `$VAR` / `$?` und entfernt Quotes |
| `heredoc.c` | Liest `<<`-Eingaben in einem Kindprozess in eine Temp-Datei |
| `exec.c` | Startet die Befehle, verbindet sie mit Pipes, wartet auf die Kinder |
| `redirect.c` | Setzt `<`, `>`, `>>` mit `open()` und `dup2()` um |
| `path.c` | Sucht Programme in `$PATH` |
| `builtins.c`, `builtins2.c` | Eingebaute Befehle |
| `env.c` | Umgebungsvariablen als verkettete Liste |
| `signals.c` | Signalhandler mit `sigaction()` |
| `utils.c` | Speicher- und String-Hilfsfunktionen |

## Implementierungsdetails

- `cd`, `export` und `exit` laufen im Hauptprozess, weil sie die Shell selbst verändern müssen.
  In einem Kindprozess würde `cd` nur das Kind betreffen. stdin/stdout werden dafür gesichert
  und danach wiederhergestellt.
- Jeder Prozess schließt die Enden, die er nicht braucht,
  sonst bekommt der Leser nie ein EOF.
- Während ein Programm läuft, ignoriert die Shell SIGINT/SIGQUIT; das Kind bekommt
  das Standardverhalten zurück. So beendet Ctrl-C nur das Programm, nicht die Shell.
- Heredocs werden in einem Kindprozess eingelesen, damit Ctrl-C nur die Eingabe abbricht und nicht die Shell beendet.
- Es gibt genau eine globale Variable (`g_signal`, `volatile sig_atomic_t`), wie im Subject vorgegeben.

## Nicht umgesetzt

`&&`, `||`, `;`, Wildcards, Subshells `( )`, `$(...)` und Job Control gehören nicht zum Pflichtteil und fehlen deshalb.
