#!/usr/bin/env bash
# download_lofi_instruments.sh
# Fetch every audio asset referenced by lofigenerator.js
# – skips files already present and prints “skip” or “done”.

set -e
EXT=webm
BASE_URL="https://lofigenerator.com"

download_file () {
  local path="$1"
  local url="${BASE_URL}/${path}"
  mkdir -p "$(dirname "$path")"
  if [[ -f "$path" ]]; then
      printf "  %-55s ... skip\n" "$path"
  else
      printf "  %-55s ... "
      if curl -fsSL "$url" -o "$path"; then
          echo "done"
      else
          echo "FAIL"
      fi
  fi
}

files=()

# ambience & vinyl
for i in {0..6}; do files+=("audio/ambience/ambience${i}.${EXT}"); done
for i in {0..3}; do files+=("audio/ambience/vinyl${i}.${EXT}");  done

# hats / kicks / snares
for i in {0..5};  do files+=("audio/hat/hat${i}.${EXT}");      done
files+=("audio/hat/shaker0.${EXT}")

for i in {0..19}; do files+=("audio/kick/kick${i}.${EXT}");    done
for i in {0..20}; do files+=("audio/snare/snare${i}.${EXT}");  done

# rhodes
files+=( audio/rhodes0/{C2,C3,C4,C5,E2,E3,E4,G2,G3,G4,G5}.${EXT} )
files+=( audio/rhodes1/{C2,C3,C4,C5,C6,C7,E1,E2,E3,E4,E5,E6,E7,Gs1,Gs2,Gs3,Gs4,Gs5,Gs6}.${EXT} )

# pianos
files+=( audio/piano_kawai/{A1,A2,A6,A7,As0,As3,As4,As5,C1,C2,C3,C4,C5,C6,C7,C8,D3,D4,D5,Ds1,Ds2,Ds6,Ds7,E3,E4,E5,Fs1,Fs2,Fs3,Fs4,Fs5,Fs6,Fs7,G7,Gs3,Gs4,Gs5}.${EXT} )
files+=( audio/piano_upright/{A2,A3,A4,A5,A6,A7,Cs1,Cs2,Cs3,Cs4,Cs5,Cs6,Cs7,C8,F1,F2,F3,F4,F5,F6,F7}.${EXT} )

# other melodic instruments
files+=( audio/kalimba/{C4,D4,Fs4,A4,C5,E5,G5,B5}.${EXT} )
files+=( audio/dantranh/{B2,B3,B4,B5,Ds3,Ds4,Ds5,Fs3,Fs4,Fs5,Gs3,Gs4,Gs5}.${EXT} )
files+=( audio/marimba/{B2,B4,C2,C4,C6,F1,F3,F5,G2,G4}.${EXT} )
files+=( audio/violin_pizz/{A3,A4,A5,C4,C5,E4,E5,G3,G4,G5}.${EXT} )
files+=( audio/uke/{A4,As5,D5,E4,G5}.${EXT} )
files+=( audio/guitar/{A3,A4,C5,E4,E5,Gs5}.${EXT} )

# basses
files+=( audio/bass/{Cs1,Fs1,B1,E2,A2,D3,G3,C4,F4,As4,Ds5,Gs5,Cs6}.${EXT} )
files+=( audio/contrabass/{As1,A2,B3,C2,Cs3,D2,E1,E2,E3,Fs1,Fs2,G1,Gs2,Gs3}.${EXT} )
files+=( audio/synthbass/{D1,Fs1,As1,D2}.${EXT} )
files+=( audio/guitar_low/{A2,C2,C3,Ds2,F2}.${EXT} )

# synth lead
files+=( audio/synth/{Cs1,Fs1,B1,E2,A2,D3,G3,C4,F4,As4,Ds5,Gs5,Cs6}.${EXT} )

printf "▶ Downloading %d asset(s)\n" "${#files[@]}"
for f in "${files[@]}"; do download_file "$f"; done
echo -e "\nAll done!"
