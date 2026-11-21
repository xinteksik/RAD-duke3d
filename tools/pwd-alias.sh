#!/bin/sh
# Physical build directories stay in the workspace; normalize only the name.
radduke_path=$(/bin/pwd -P)
case "$radduke_path" in
  "$RADDUKE_PHYSICAL_ROOT"*) printf '%s%s\n' /tmp/radduke-work "${radduke_path#"$RADDUKE_PHYSICAL_ROOT"}" ;;
  *) printf '%s\n' "$radduke_path" ;;
esac
