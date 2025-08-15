#!/usr/bin/env bash
# fix-locale.sh — make a UTF-8 locale available and active system-wide
# Usage: bash fix-locale.sh [LOCALE]
# Example: bash fix-locale.sh en_US.UTF-8   or   bash fix-locale.sh C.UTF-8

set -euo pipefail

DESIRED_LOCALE="${1:-en_US.UTF-8}"

is_cmd() { command -v "$1" >/dev/null 2>&1; }
need_sudo() { [ "$(id -u)" -eq 0 ] || echo sudo; }
SUDO="$(need_sudo || true)"

have_locale() { locale -a 2>/dev/null | grep -qiE "^${1}$"; }

ensure_debian_like() {
  $SUDO apt-get update -y
  $SUDO apt-get install -y locales
  if [ -f /etc/locale.gen ]; then
    # ensure the line exists and is uncommented
    esc="${DESIRED_LOCALE//./\\.}"
    if ! grep -qE "^${esc}" /etc/locale.gen; then
      echo "${DESIRED_LOCALE} UTF-8" | $SUDO tee -a /etc/locale.gen >/dev/null
    fi
    $SUDO sed -ri "s/^# *(${esc}).*/\1 UTF-8/" /etc/locale.gen || true
  fi
  $SUDO locale-gen "${DESIRED_LOCALE}" || true
  $SUDO update-locale LANG="${DESIRED_LOCALE}" LC_ALL="${DESIRED_LOCALE}" || true
}

ensure_alpine() {
  # musl: provide locales package; C.UTF-8 is usually available after this
  $SUDO apk add --no-cache musl-locales musl-locales-lang || true
}

ensure_rhel_like() {
  local PKG=dnf
  is_cmd dnf || PKG=yum
  # glibc-langpack provides prebuilt locales on modern RHEL/Fedora
  $SUDO $PKG install -y glibc-langpack-en glibc-common || true
  # Fallback: generate desired locale if needed
  $SUDO localedef -i "${DESIRED_LOCALE%%.*}" -f UTF-8 "${DESIRED_LOCALE}" || true
  echo "LANG=${DESIRED_LOCALE}" | $SUDO tee /etc/locale.conf >/dev/null
}

ensure_arch() {
  $SUDO pacman -Sy --noconfirm glibc
  esc="${DESIRED_LOCALE//./\\.}"
  if ! grep -qE "^${esc}" /etc/locale.gen; then
    echo "${DESIRED_LOCALE} UTF-8" | $SUDO tee -a /etc/locale.gen >/dev/null
  fi
  $SUDO sed -ri "s/^# *(${esc}).*/\1 UTF-8/" /etc/locale.gen || true
  $SUDO locale-gen
  echo "LANG=${DESIRED_LOCALE}" | $SUDO tee /etc/locale.conf >/dev/null
}

ensure_unknown() {
  # Best-effort for unknown distros
  $SUDO localedef -i "${DESIRED_LOCALE%%.*}" -f UTF-8 "${DESIRED_LOCALE}" || true
}

echo ">> Target locale: ${DESIRED_LOCALE}"

if ! have_locale "${DESIRED_LOCALE}"; then
  if   is_cmd apt-get; then ensure_debian_like
  elif is_cmd apk;     then ensure_alpine
  elif is_cmd dnf || is_cmd yum; then ensure_rhel_like
  elif is_cmd pacman;  then ensure_arch
  else ensure_unknown
  fi
fi

# If still missing, try C.UTF-8 (widely available) before giving up.
if ! have_locale "${DESIRED_LOCALE}"; then
  echo ">> '${DESIRED_LOCALE}' not found after install. Falling back to C.UTF-8."
  DESIRED_LOCALE="C.UTF-8"
fi

# System-wide env exports
if [ -d /etc/profile.d ]; then
  $SUDO bash -c "cat > /etc/profile.d/locale.sh" <<EOF
export LANG=${DESIRED_LOCALE}
export LC_ALL=${DESIRED_LOCALE}
export LANGUAGE=${DESIRED_LOCALE%.*}:${DESIRED_LOCALE%%_*}
EOF
  $SUDO chmod 0644 /etc/profile.d/locale.sh
fi

# For current shell/session
export LANG="${DESIRED_LOCALE}"
export LC_ALL="${DESIRED_LOCALE}"
export LANGUAGE="${DESIRED_LOCALE%.*}:${DESIRED_LOCALE%%_*}"

echo ">> Verifying..."
locale || true
perl -e 'print "Perl OK\n"' >/dev/null 2>&1 && echo ">> Perl OK"

echo ">> Locale configured: LANG=${LANG}, LC_ALL=${LC_ALL}"
echo ">> Reopen your shell or 'source /etc/profile' if needed."
