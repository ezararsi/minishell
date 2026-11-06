#!/usr/bin/env bash
# Vergleicht Ausgabe und Exit-Code von minishell mit bash.
# Aufruf: make test   (oder: bash tests/run_tests.sh)
# Mehrere Befehle pro Test stehen in eigenen Zeilen, da die Shell kein ';' kennt.

cd "$(dirname "$0")/.." || exit 1
MS="$(pwd)/minishell"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
pass=0
fail=0

t() {
    local o1 o2 s1 s2
    rm -rf "$TMP/ms" "$TMP/bash"
    mkdir -p "$TMP/ms" "$TMP/bash"
    o1=$(cd "$TMP/ms" && printf '%s\n' "$1" | env -i PATH="$PATH" HOME="$HOME" USER=test "$MS" 2>/dev/null)
    s1=$?
    o2=$(cd "$TMP/bash" && printf '%s\n' "$1" | env -i PATH="$PATH" HOME="$HOME" USER=test bash --norc --noprofile 2>/dev/null)
    s2=$?
    if [[ "$o1" == "$o2" && $s1 == "$s2" ]]; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1))
        printf 'FAIL: %s\n  minishell [%d]: %q\n  bash      [%d]: %q\n' "$1" "$s1" "$o1" "$s2" "$o2"
    fi
}

# echo und Quotes
t 'echo hello world'
t 'echo -n hi'
t 'echo -nnn -n x'
t "echo 'single \$HOME' \"double \$HOME\""
t 'echo "a   b"   c'
t 'echo a"b"c'"'"'d'"'"
t 'echo ""'

# Variablen
t 'echo $USER'
t 'echo $NOPE end'
t 'echo "$NOPE" | cat -e'
t 'echo $?'
t 'false
echo $?'
t 'export X=42
echo $X'
t 'export X=1
unset X
echo [$X]'
t 'export A="hallo welt"
echo "$A"'

# Pipes
t 'echo eins | cat'
t 'printf "b\na\nc\n" | sort | head -2'
t 'ls / | grep -c bin'
t 'cat /etc/hostname | wc -l'
t 'echo x | false'
t 'false | echo y'

# Umleitungen
t 'echo hallo > out
cat out'
t 'echo a > f
echo b >> f
cat f'
t 'echo text > f
cat < f'
t 'echo x > a > b
cat a b'
t 'cat < gibtsnicht
echo $?'
t 'cat << EOF
zeile $USER
EOF'
t "cat << 'EOF'
zeile \$USER
EOF"

# Builtins
t 'cd /tmp
pwd'
t 'cd /gibtsnicht
echo $?'
t 'env | grep -c ^USER='
t 'exit 7'
t 'exit 256'
t 'exit -1'

# Fehler und Exit-Codes
t 'gibtsnicht
echo $?'
t '/etc
echo $?'
# bash bricht ohne Terminal bei Syntaxfehlern ab, deshalb nur als letzte Zeile
t '| echo x'
t 'echo >'
t 'cat <<'
t 'echo "offen'

printf '\n%d bestanden, %d fehlgeschlagen\n' "$pass" "$fail"
[[ $fail -eq 0 ]]
