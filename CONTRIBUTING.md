# Contributing to Super Mario Strikers Viewer

Thank you for your interest in contributing to this project!

## Important Guidelines

### Asset Policy

**CRITICAL**: Never commit or include any copyrighted game assets in pull requests:
- No game files, ROM files, or ISO images
- No textures, models, or animations extracted from the game
- No audio files from the original game
- No binary data from the game

### Code of Conduct

- Be respectful and professional
- Focus on constructive feedback
- Help maintain a welcoming environment

## How to Contribute

### Reporting Issues

When reporting bugs or suggesting features:
1. Check if the issue already exists
2. Provide detailed reproduction steps
3. Include system information (OS, compiler, versions)
4. Attach relevant logs or screenshots (without game assets)

### Submitting Pull Requests

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes
4. Ensure code compiles on multiple platforms (if possible)
5. Follow the existing code style
6. Write meaningful commit messages
7. Push to your fork
8. Open a Pull Request

### Code Style

- Use C++17 features appropriately
- Follow consistent naming conventions:
  - Classes: `PascalCase`
  - Functions/methods: `camelCase`
  - Variables: `camelCase`
  - Constants: `UPPER_SNAKE_CASE`
  - Private members: `m_camelCase`
- Add comments for complex logic
- Document public APIs with Doxygen-style comments

### Testing

- Test your changes on your platform
- If possible, test on multiple platforms
- Document any platform-specific behavior

### Documentation

- Update README.md if adding new features
- Add or update code comments
- Update docs/ if changing architecture

## Development Setup

See [README.md](../README.md) for build instructions.

## Questions?

Feel free to open an issue for questions or discussions about:
- Feature ideas
- Implementation approaches
- Technical design decisions

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
