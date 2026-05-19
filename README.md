# Extended Drive - Disk Operating System (xD-DOS)

xD-DOS is an operating system inspired by **DOS**. xD-DOS is merely a fun project and **SHOULD NOT** be used for serious terms.

* **Inspired, not rebuilt:** xD-DOS does not aim to be a remake of DOS, but only inspired by DOS.
* **Modular:** Designed with every component of the OS being separate applications instead of one big kernel.

## Checklist

* ~~Configure limine~~
* ~~Set up kernel entry point~~
* ~~Write to framebuffer~~
* Set up a global descriptor table (GDT)
* Set up an interrupt descriptor table (IDT)
* Write assembly interrupt stubs
* Handle CPU exceptions
* Write basic drivers
* Build a simple command line

## Getting Started

### Prerequisites
* make
* qemu-system-x86_64 (optional for emulating)

### Installation / Compilation
1. **Clone the repository**:
```bash
git clone https://github.com/jasonchristiandev/xD-DOS.git
```

2. **Build the OS**:
```bash
cd xD-DOS
make
```

3. **Run the OS using qemu-system-x86_64 (optional)**:
```bash
make clean # optional for clearing cache
make run
```

## License

xD-DOS is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for more detail.

## Contributing

Contributions are encouraged, due to my lack of experience in OS dev :p

### How to Contribute

1. **Fork the repository**: Create your own copy of the project to work on.

2. **Create a feature branch**:
```bash
git checkout -b feature/YourFeatureName
```

3. **Commit your changes**: 
```bash
git commit
```

4. **Push and pull request**: Push your branch to GitHub and open a pull request. Descriptions are very appreciated.
