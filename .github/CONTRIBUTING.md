# Contribution Guidelines

Contributions to Acer Nitro Sense Linux are welcome. Bugs, model profiles,
documentation fixes, cleanup, and carefully validated hardware support are all
useful.

For larger changes, especially anything that writes EC registers or changes fan
control behavior, please open an issue or message me on Discord
(**almightyshogun**) first so the approach can be discussed before code is
written.

# 🧹 Coding Style

This project follows the existing C style in `src/` and keeps code split by
area: client, commands, config, daemon, EC access, fan control, hardware,
keyboard, platform, sensors, and utilities.

Keep changes focused and avoid unrelated refactors in feature or bugfix pull
requests. Hardware-facing code should be explicit, defensive, and easy to audit.

# 🔒 Hardware Safety

EC writes can affect cooling and platform behavior. Do not add or change EC
register writes unless they have been validated on real hardware or isolated
behind a model profile that prevents accidental use on unrelated laptops.

Model support must include:

- DMI allow-list values
- validated read/write registers
- safe reset behavior
- validation notes or output from the hardware scripts in `scripts/`

See `docs/adding-models.md` for the expected model validation flow.

# 🧪 Testing

Before opening a pull request, run the relevant local checks:

```sh
meson setup build --prefix=/usr --sysconfdir=/etc
meson compile -C build
meson test -C build
```

For hardware or model changes, also run the relevant validation script from
`scripts/` and include the summary path or important output in the pull request.

# 🪵 Git
All commit messages, branches, issues and/or pull requests will be in English.

### Branches
- **Main** — This is the `main` branch. This contains the latest stable release and is the exact source running in production.
- **Development** — This is the `development` branch. This contains the latest staging release that is marked for deployment and is the exact source running on staging.
- **Feature** — This is a `feature/*` branch. This contains a new feature that will be added. Any feature should have its own branch. Once completed the branch should be merged into the `development` branch.
- **Bugfix** — This is a `bugfix/*` branch. This contains a bugfix that will be added. Any bugfix should have its own branch. Once completed the branch should be merged into the `development` branch.

### Commit messages
Commit messages are bound to the following template:
- `<type>: <message>`
- `<type>(feature): <message>`
- `<type>(feature): <message> [<issue-number>]`

### Examples
Some examples of a commit message:
- `feat(models): add support for Nitro AN515-58`
- `fix(ec): restore firmware fan mode after shutdown`
- `docs: clarify release installation steps`
