# GIT AI hooks

This repository contains various git helper hooks with ChatGPT assisted productivity tooling.

## Installation

Build project with classic cmake commands:

```bash
mkdir build
cd build
cmake ..
make -j12
```

Set your OpenAI key:

>./GitAiHooks --set-api-key "sk-..."

## Commit message helper hook

If you want to use message helper only for one repository, copy it to your repo's hook directory:

```bash
cp GitAiHooks .git/hooks/prepare-commit-msg
```

From now on, you should be suggested with proper git commit messages according to your stashed changes.

Also you can install hook for all projects in your machine:

```bash
mkdir ~/sources/.githooks
cp GitAiHooks ~/sources/.githooks
git config --global core.hooksPath ~/sources/.githooks
```

Enjoy.

