# Palpick - a terminal color picker
## Usage

### Controls
- **`h` `j` `k` `l`** — Move the cursor to select a hue or value  
- **`m`** — Toggle between hue selection and value selection  
- **`Enter`** — Select the color and exit  

---

## Options

| Option | Description |
|------|------------|
| `-h`, `--help` | Print this help message |
| `--x`, `--y` | X/Y origin (upper-left corner) of the widget. Defaults to `1` |
| `--width`, `--height` | Width and height of the hue bar and gradient box (excluding right-side info text). Defaults to `20` and `10` |
| `--prefix` | Prepends the final output with this string |
| `--postfix` | Appends the final output with this string |
| `--separator1` | Separator between the red and green components. Defaults to `,` |
| `--separator2` | Separator between the green and blue components. Defaults to `,` |
| `--red-label` | Label before the red component |
| `--green-label` | Label before the green component |
| `--blue-label` | Label before the blue component |

---

## Examples

```sh
palpick --pre "Vec3(" --post ")" --separator1 ", " --separator2 ", "
# Output:
# Vec3(123, 34, 105)
```

```sh
palpick --separator1 "; " --separator2 "; " \
          --red-label "R: " \
          --green-label "G: " \
          --blue-label "B: "

# Output:
# R: 123; G: 34; B: 105
```

## Installation guide 
### Build from source
No shared libs need to be linked. All dependecies are part of the project.
```bash
# cd to your folder
git clone https://github.com/paunescudanutz/pal-pick.git
gcc src/*.c -lm -o bin/palpick
```
Your executable should be found at `<root of directory>/bin/palpick`
