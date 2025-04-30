#!/bin/bash

VERSION_FILE="version.h"

# Get the latest Git tag
LATEST_TAG=$(git describe --tags --abbrev=0 2>/dev/null)

# Get the short commit hash of the current HEAD
COMMIT_HASH=$(git rev-parse --short HEAD 2>/dev/null)

# Check if the working directory has uncommitted changes
if [ -n "$(git status --porcelain)" ]; then
    DIRTY="-dirty"
else
    DIRTY=""
fi

# Fallback if no tags exist
if [ -z "$LATEST_TAG" ]; then
    LATEST_TAG="0.0.0"
fi

# Construct the version string
VERSION="${LATEST_TAG}-${COMMIT_HASH}${DIRTY}"

# Check existing version if version.h exists
if [ -f "$VERSION_FILE" ]; then
    EXISTING_VERSION=$(grep 'VERSION_STRING' "$VERSION_FILE" | cut -d '"' -f2)
else
    EXISTING_VERSION=""
fi

# Only regenerate if version changed
if [ "$VERSION" != "$EXISTING_VERSION" ]; then
    cat <<EOF > "$VERSION_FILE"
#ifndef __MORDROREUR_VERSION_H__
#define __MORDROREUR_VERSION_H__

#define VERSION_STRING "${VERSION}"

#endif // __MORDROREUR_VERSION_H__
EOF

    echo "version.h generated with VERSION_STRING: ${VERSION}"
else
    echo "version.h is up-to-date (VERSION_STRING: ${VERSION})"
fi
