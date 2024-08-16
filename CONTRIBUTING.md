## Contribution guidelines

An open-source project thrives on the love of the community. `cbt` is no different.

As an author and maintainer of `cbt`, I hope to see this project have a big and positive impact on the world. As such, feedbacks/inputs are always welcome!

### Expectations

Before we move ahead, I humbly ask the community to understand and respect the following:

1. `cbt` is on a mission to simplify application/library development in `C++` using only commands. It **should not** deviate from the original [Philosophy](README.md#philosophy). References from other project(s) are welcome, and may be incorporated, provided they don't dilute the very fundamentals or true identity of `cbt`.
2. Unlike other projects, `cbt` in its own rights, will head towards feature-completeness, i.e. a stage when *there is nothing to add, but nothing to take away*. If it is observed at a future point in time, that `cbt` can do all that was originally envisioned: except for bug fixes, development should cease. **A dormant project does not necessarily mean it is of no value, or that it cannot stand the test of time.** If that's the case, an [Announcement](https://github.com/swar-mukh/cbt/discussions/categories/announcements) will be made. Fret not!
3. It is only human to have varying perspectives; differences in opinion(s) can arise. While I do respect opinion(s) of others, continuing on the above points, it may so happen that such changes may not be incorporated, or that it may require substantial tweaks before acceptance.
4. `cbt` is not released in regular intervals but rather at a steady cadence. Release happens when there's an urgent bug-fix made or when substantial improvements/features have been added, or a mix of the two. Most of the time is spent on careful planning and prevention of scope-creep. In mentioning this, I must also point out that I work on `cbt` in the very limited time I get after my professional and personal commitments. I often check the repository, but there *may* be periods of slight delay. I hope the community will understand.

### Boundaries

1. **Law before code/community**: Legal attributions are to be upheld at all cost. Intentions are to be clearly communicated. Conversely, inferring a logic from such intentions is paramount and equally a prerequisite. If any code contribution/change infringes on any open-source licensing clause or on requirements as per law, immediately call it out.
2. **Respecting fellow human-beings**: Charity begins at home. There shall be no negative engagements either in code, or in discussions, relating to core human attributes, i.e. sex, race, sexual orientation, caste, nationality, political orientation, disability or any reference to an individual's personal life. Be professional and courteous.
3. **Zero dependencies**: `cbt` is a torch-bearer on how far a project can come to becoming functional without any dependencies. The only dependency is the standard library. Whilst `cbt` has the capability of bootstrapping itself, it will **not** use any third-party dependency to enhance its functionalities. If an existing functionality offered by the standard library is not enough or contains a bug which hasn't been addressed yet, rather than developing in silos, we must holistically engage with the ISO committee for C++ and push forward the state of the art.
4. **Platform agnosticism**: No platform (and their users) shall be mistreated. Consequently, there shall be no preferential treatment towards any platform. Unless a bug or an enhancement or a piece of functionality cannot be adequately addressed for a particular platform, any such changes must adequately address all platforms. Leverage standard libraries as much as possible.

### Contribution

Here are few ways you can contribute:

1. The best places to begin are either the [Issues](https://github.com/swar-mukh/cbt/issues) section (both `open` and [closed](https://github.com/swar-mukh/cbt/issues?q=is%3Aissue+is%3Aclosed) issues) or the [Discussions](https://github.com/swar-mukh/cbt/discussions) section. Please check if your query/concern is addressed or referenced in those two sections. If not, move on to the next pointers below.
2. If something is unclear (e.g. context, purpose of a piece of code, etc.), or cannot be found in the codebase, please raise a [question](https://github.com/swar-mukh/cbt/discussions/categories/q-a) (found under [Discussions](https://github.com/swar-mukh/cbt/discussions) section). I'll be happy to address such queries to the best of my knowledge.
3. Similar to point 2 above, make use of the appropriate discussion type (under `Discussions` section) that GitHub provides, to drive engagement with the community.
4. Before submitting a pull-request (PR), ensure you have checked the following:
   - Whether there is a similar [PR](https://github.com/swar-mukh/cbt/pulls) being addressed or has been addressed
   - Whether there is a feature/fix being currently worked on in any [branch](https://github.com/swar-mukh/cbt/branches), or is part of an existing PR
5. If none of the above is/are satisfactory, and if you think that there's a substantial impact of your contribution through which the community can benefit, [fork](https://github.com/swar-mukh/cbt/fork) this repository (if not already done), add your contributions and raise a PR. Ensure you have followed the [style-guide](#style-guide) below.
6. If you find that my own contributions contradict the style-guide, don't hesitate to point it out.

### Style guide

***Note:** This section applicable only for code contributions. (To be continuously updated)*

1. Use idiomatic C++ as much as possible. `C++2a` is the base standard.
2. Use `.hpp` and `.cpp` extensions for header and implementation files, respectively. Usage of any other extension(s) will **not** be entertained.
3. A header file *may not* have a corresponding implementation file; but an implementation file **must** have a corresponding header file.
4. No private attributes/functionalities/implementations should be present in header files. All such private entities **must** be in un-named `namespace`s in the corresponding implementation file.
5. Directory nesting plays a major role in `cbt` w.r.t. the structure of code. Removing cognitive load is the primary motto of `cbt`, hence nesting must also be reflected in the code as follows:
   - All header files must be guarded, i.e. if the file is `headers/a/b/c/d.hpp`, then its guard should be `A_B_C_D`.
   - If a file-pair is located in `headers/a/b/c/d.hpp` and `src/a/b/c/d.cpp` then its prototypes/declarations and definitions respectively, should be wrapped in `namespace a::b::c::d {}`.
   - Do **not** open/extend a namespace in different files or at different depths, i.e. one case provided as follows: if files `src/a/b/c.cpp` and `src/a/b/c/d.cpp` are present, then do not modify `namespace a::b::c {}` in `src/a/b/c/d.cpp`, or any other combinations thereof.
6. Use `<Tab>` instead of `<Space>` for indentation.
7. One-liner loop bodies and/or conditionals should be enclosed in braces regardless.
