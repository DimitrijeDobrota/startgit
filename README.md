# startgit

Static page generator for git repositories

## Description

With this project you can seamlessly turn a git repositories into a static
webpage, with auto generated indexes, diffs, commits and files for for each
branch.

The big source of inspiration for this project is [`Stagit`](https://codemadness.org/git/stagit/log.html). I've copied 
most, if not all, of the functionality, with a few twists to fit my needs
better and fit with the reset of the aesthetic.

It's configured to make pages for my [`personal projects`](https://git.dimitrijedobrota.com/),
but as generation code is self documenting , it can be easily adopted to fit
your needs.

As of now, md4c library is used to render markdown files in html for specific
pages in order to improve readability, until I find time to develop
sophisticated Markdown to HTML converter.


## Dependencies

* CMake 3.14 or latter
* Compiler with C++20 support (tested: clang 18.1.8, gcc 14.2.1)
* [`Git2Wrap 0.1.0`](https://github.com/DimitrijeDobrota/gitwrap)
* [`Poafloc 1.0.0`](https://github.com/DimitrijeDobrota/poafloc)
* [`Hemplate 0,2,2`](https://github.com/DimitrijeDobrota/hemplate)
* [`md4c`](https://github.com/mity/md4c)


## Building and installing

See the [`BUILDING`](BUILDING.md) document.


## Usage

> Run `stargti --help` for the explanation of command line arguments


## Version History

- 0.2.1
    * Remove branch dropdown
    * Handle only one repository
    * Separate indexer
- 0.2
    * Files, commits and diffs for each branch
    * RSS and Atom feeds per branch
    * On demand evaluation
    * Github to local link translation
- 0.1
    * Initial development phase


## Contributing

See the [`CONTRIBUTING`](CONTRIBUTING.md) document.


# Licensing

This project is licensed under the MIT License
- see the [`LICENSE`](LICENSE.md) document for details


## Acknowledgments

Inspiration, code snippets, etc.
* [`Stagit`](https://codemadness.org/git/stagit/log.html)
