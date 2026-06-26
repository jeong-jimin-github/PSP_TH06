#!/bin/bash
set -e

# Usage: ./scripts/build_psp_mac.sh <SourceDir> [PspdevDir]
SOURCE_DIR="$1"
PSPDEV_DIR="${2:-$HOME/pspdev}"

if [ -z "$SOURCE_DIR" ]; then
    echo "Usage: $0 <SourceDir> [PspdevDir]"
    exit 1
fi

# 1. Validate paths
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory '$SOURCE_DIR' does not exist."
    exit 1
fi

# Check original game data
REQUIRED_DATA=("CM.DAT" "ED.DAT" "IN.DAT" "MD.DAT" "ST.DAT" "TL.DAT")
for file in "${REQUIRED_DATA[@]}"; do
    if [ ! -f "$SOURCE_DIR/$file" ]; then
        echo "Error: Missing original game data: $SOURCE_DIR/$file"
        exit 1
    fi
done

# Check BGM
BGM_COUNT=$(ls -1 "$SOURCE_DIR/bgm"/th06_*.wav 2>/dev/null | wc -l)
if [ "$BGM_COUNT" -ne 17 ]; then
    echo "Error: Expected 17 original BGM WAV files in $SOURCE_DIR/bgm, found $BGM_COUNT."
    exit 1
fi

# 2. Build PSP Project
export PSPDEV="$PSPDEV_DIR"
export PATH="$PSPDEV/bin:$PATH"

if ! command -v psp-cmake &> /dev/null; then
    echo "Error: psp-cmake not found. Please install pspdev first."
    exit 1
fi

echo "==> Configuring and building PSP project..."
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=Release ${EXTRA_CMAKE_ARGS}
cmake --build build/psp -j$(sysctl -n hw.ncpu)

# 3. Build Host Extractor Tool
echo "==> Building host extractor tool..."
mkdir -p build/host-tools
g++ -std=c++20 -O2 -Isrc tools/pbg3_extract.cpp src/pbg3/FileAbstraction.cpp src/pbg3/IPbg3Parser.cpp src/pbg3/Pbg3Archive.cpp src/pbg3/Pbg3Parser.cpp -o build/host-tools/pbg3_extract

# 4. Setup package directory
echo "==> Setting up package directory..."
PACKAGE_DIR="build/psp-package/PSP/GAME/TH06"
rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/data"
mkdir -p "$PACKAGE_DIR/bgm"
mkdir -p "$PACKAGE_DIR/replay"

# 5. Extract assets
echo "==> Extracting assets with host extractor..."
for name in "${REQUIRED_DATA[@]}"; do
    archive_path="$SOURCE_DIR/$name"
    archive_stem="${name%.*}"
    asset_dir="$PACKAGE_DIR/data/$archive_stem"
    mkdir -p "$asset_dir"
    
    ./build/host-tools/pbg3_extract "$archive_path" "$asset_dir"
    if [ $? -ne 0 ]; then
        echo "Error: Failed to extract $name"
        exit 1
    fi
done

# 6. Copy build output and original assets
echo "==> Copying EBOOT.PBP and assets..."
cp "build/psp/EBOOT.PBP" "$PACKAGE_DIR/"
for name in "${REQUIRED_DATA[@]}"; do
    cp "$SOURCE_DIR/$name" "$PACKAGE_DIR/"
done
cp "$SOURCE_DIR/bgm"/th06_*.wav "$PACKAGE_DIR/bgm/"

# 7. Font setup
# Look for a font in $SOURCE_DIR, macOS system, or repository root
FONT_FILE=""
if [ -f "$SOURCE_DIR/msgothic.ttc" ]; then
    FONT_FILE="$SOURCE_DIR/msgothic.ttc"
elif [ -f "$SOURCE_DIR/NotoSansJP-Regular.ttf" ]; then
    FONT_FILE="$SOURCE_DIR/NotoSansJP-Regular.ttf"
elif [ -f "/System/Library/Fonts/Supplemental/MS Gothic.ttf" ]; then
    FONT_FILE="/System/Library/Fonts/Supplemental/MS Gothic.ttf"
elif [ -n "$(find /System/Library/Fonts -maxdepth 1 -name "*角*シック*W3.ttc" -print -quit 2>/dev/null)" ]; then
    FONT_FILE=$(find /System/Library/Fonts -maxdepth 1 -name "*角*シック*W3.ttc" -print -quit 2>/dev/null)
elif [ -n "$(find /System/Library/Fonts -maxdepth 1 -name "*角*シック*.ttc" -print -quit 2>/dev/null)" ]; then
    FONT_FILE=$(find /System/Library/Fonts -maxdepth 1 -name "*角*シック*.ttc" -print -quit 2>/dev/null)
elif [ -f "NotoSans-Regular.ttf" ]; then
    FONT_FILE="NotoSans-Regular.ttf"
fi

if [ -z "$FONT_FILE" ]; then
    echo "Error: No Japanese font found. Put msgothic.ttc or NotoSansJP-Regular.ttf in $SOURCE_DIR, or install MS Gothic on your system."
    exit 1
fi

FONT_NAME=$(basename "$FONT_FILE")
if [ "$FONT_NAME" = "NotoSansJP-Regular.ttf" ]; then
    TARGET_FONT_NAME="NotoSansJP-Regular.ttf"
elif [ "$FONT_NAME" = "NotoSans-Regular.ttf" ]; then
    TARGET_FONT_NAME="NotoSansJP-Regular.ttf"
else
    TARGET_FONT_NAME="msgothic.ttc"
fi

echo "==> Copying font $FONT_FILE as $TARGET_FONT_NAME..."
cp "$FONT_FILE" "$PACKAGE_DIR/$TARGET_FONT_NAME"

echo "==> PSP package ready: $PACKAGE_DIR"
PACKAGE_SIZE=$(du -sh "$PACKAGE_DIR" | cut -f1)
echo "Package size: $PACKAGE_SIZE"
