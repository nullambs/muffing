# muffing
[MU]lti-[F]eatured [F]iles serving th[ING]

a lightweight file server written in nobody
[C]ares what for HTTP ~~FTP~~ ~~TELNET~~ ~~SSH~~
...you just basically observed the roadmap;;

## installation

### windows/macos
```bash
echo "muffing is up and ready to accept connections"
echo "  HTTP access points"
echo "  - localhot     http://localhost:8080"
echo "  - lan          http://<IP>:8080"
```

### linux

```bash
git clone https://github.com/nullambs/muffing.git
cd muffing
make build
sudo cp build/muffing /usr/local/bin/
```

## Usage

```bash
muffing -h # for options i don't want to repeat here
```

## Building

Requirements:
- GCC or compatible C compiler
- Make

```bash
make build
```

## Development

```bash
# Build in debug mode
make build DEBUG=true

# Clean build artifacts
make clean
```

## License

This project is licensed under the GNU General Public License v3.0 - see the LICENSE file for details.
