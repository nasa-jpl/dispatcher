# Icons

All icons in this directory are embedded within the `dispatcher` executable and
can be loaded using Qt functions. For example, to load the red.png file as an
image:

```
QImage(":/icons/red.png")
```

You can also use these icons for custom scripts. For example:

```
scripts:

  - name: htop
    cmd: htop
    icon: :/icons/terminal.png
    row: 0
    column: 0
    use_terminal: true
```
