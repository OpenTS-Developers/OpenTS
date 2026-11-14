"""Keep a test's git commands inside its temporary repository."""

import os
import subprocess
from unittest import mock


def isolate(test):
    """Remove the variables that select a repository until ``test`` ends.

    A test run from a git hook inherits ``GIT_DIR``, which overrides the
    working directory its git commands run in.
    """

    listing = subprocess.run(
        ["git", "rev-parse", "--local-env-vars"],
        text=True, stdout=subprocess.PIPE, check=True)
    patch = mock.patch.dict(os.environ)
    patch.start()
    test.addCleanup(patch.stop)
    for name in listing.stdout.split():
        os.environ.pop(name, None)
