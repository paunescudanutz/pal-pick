palpick - a terminal color picker

Usage:
    h, j, k, l - used to move the cursor for selecting a hue or a value
    m - toggle between selecting a hue or selecting a value
    Enter - select the color and exit

Options:
    -h, --help           Prints this help
    --x, --y             X/Y origin (upper left hand corner) of widget. Defaults to 1
    --width, --height    Width and Height of the Hue Bar and Gradient box - not including the right side info text. Defaults to 20 and 10
    --prefix             Prepends the final output with this string
    --postfix            Appends to the final output with this string
    --separator1         The char or string to use as the separator between the Red and Green components. Defaults to ','
    --separator2         The char or string to use as the separator between the Green and Blue components. Defaults to ','
    --red-label          The label before the Red component
    --green-label        The label before the Gree component
    --blue-label         The label before the Blue component

Examples:
    <command> --pre "Vec3(" --post ")" --separator1 ", " --separator2 ", " --> Vec3(123, 34, 105)
    <command> --separator1 "; " --separator2 "; " --red-label "R: " --green-label "G: " --blue-label "B: " --> R: 123; G: 34; B: 105
