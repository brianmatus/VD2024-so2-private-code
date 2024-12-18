#!/bin/bash


# Update accordingly
ORIGINAL_KERNEL_DIR=~/linux-6.8
CUSTOM_KERNEL_DIR=$(pwd)


# Do not modify this ones
ORIGINAL_KERNEL_DIR=$(realpath "$ORIGINAL_KERNEL_DIR")
CUSTOM_KERNEL_DIR=$(realpath $CUSTOM_KERNEL_DIR)

# Checks if a path resides inside a symlinked directory until ORIGINAL_KERNEL_DIR.
# This because all files that resided in a new custom directory will not be detected as symlinks
# because the actual symlink is one of the parent directories.
is_in_symlinked_dir() {
  local path="$1"
  while [ "$path" != "$ORIGINAL_KERNEL_DIR" ]; do
    if [ -L "$path" ]; then
      return 0 # True, the path resides in a symlinked directory
    fi
    path=$(dirname "$path")
  done
  return 1 # False, no symlinked directories in the hierarchy
}

# To exclude the script itself (since its also git-tracked)
SCRIPT_NAME=$(basename "$0")


# Iterate over all Git-tracked files
git ls-files | while read -r file; do
  RELATIVE_PATH=$(realpath --relative-to="$CUSTOM_KERNEL_DIR" "$file")
  ORIGINAL_PATH="$ORIGINAL_KERNEL_DIR/$RELATIVE_PATH" # appends the original-kernel folder to make it absolute

  # Skip the script itself
  if [ "$RELATIVE_PATH" == "$SCRIPT_NAME" ]; then
    continue
  fi

  if is_in_symlinked_dir "$(dirname "$ORIGINAL_PATH")"; then
    # echo "Skipping $file because it resides in a symlinked directory." # TODO uncomment if wanted
    continue
  fi

  # Check if the symlink exists and points to the correct target
  if [ -e "$ORIGINAL_PATH" ]; then
    if [ -L "$ORIGINAL_PATH" ]; then
      # Correct target?
      if [ "$(readlink -f "$ORIGINAL_PATH")" == "$CUSTOM_KERNEL_DIR/$RELATIVE_PATH" ]; then
        continue # Symlink already exists and is correct
      else
        # Symlink exists but points to the wrong location, removing it
        rm "$ORIGINAL_PATH"
      fi
    else
      # Regular file, cannot replace it
      echo "Error: $ORIGINAL_PATH exists and is not a symlink. Skipping."
      continue
    fi
  fi

  # Ensure the target directory exists in the original kernel folder
  mkdir -p "$(dirname "$ORIGINAL_PATH")"

  # Create the symlink
  ln -s "$CUSTOM_KERNEL_DIR/$RELATIVE_PATH" "$ORIGINAL_PATH"
  echo -e "\e[32m$ORIGINAL_PATH -> $CUSTOM_KERNEL_DIR/$RELATIVE_PATH\e[0m"

done

echo "done"

