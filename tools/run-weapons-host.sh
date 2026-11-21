#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")/.."
repo="$PWD";run="$repo/${RADDUKE_WEAPON_RUN:-build/weapons-host-run}"
test ! -e "$run" || { echo 'Refusing to overwrite weapon evidence';exit 2; }
mkdir -p "$run"
cp SD/RADDUKE/duke3d.grp "$run/duke3d.grp"
cp SD/RADDUKE/AUDIO.CFG "$run/AUDIO.CFG"
touch "$run/user_profiles_disabled"
cd "$run"
export SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy RADDUKE_GAMEPLAY=1 RADDUKE_WEAPON_TEST=1
export RADDUKE_SOUNDFONT="$repo/build/music-assets/soundfont.sf2"
"$repo/build/effects-host/radduke-host" > host.log 2>&1
echo WEAPON_HOST_RUN_OK
