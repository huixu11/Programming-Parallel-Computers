Use **WSL Ubuntu** on Windows. The course grading tool is designed for Linux/macOS: the page says the zip includes a `./grading` tool that should work in typical Linux/macOS environments, and you run tests with `./grading test` / benchmarks with `./grading benchmark`. ([ppc-exercises.cs.aalto.fi][1])

## 1. Install WSL Ubuntu

Open **PowerShell as Administrator**:

```powershell
wsl --install -d Ubuntu
```

Restart if Windows asks. Then open **Ubuntu** from Start Menu.

## 2. Install C++ tools inside Ubuntu

```bash
sudo apt update
sudo apt install -y build-essential g++ clang make cmake python3 unzip git
```

## Common Windows/WSL errors

If you get:

```text
Permission denied
```

run:

```bash
chmod +x grading
```

If you get:

```text
g++: command not found
```

run:

```bash
sudo apt install -y build-essential
```

If you get a weird line-ending error like:

```text
bad interpreter: /bin/bash^M
```

run:

```bash
sudo apt install -y dos2unix
dos2unix grading
```

Then run again:

```bash
./grading test
```

For this course, I would not try to run `./grading` directly in Windows CMD/PowerShell. Use WSL.
