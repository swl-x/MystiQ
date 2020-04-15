# Developer Process & Workflow

This document defines the process and workflow as per github and gitflow good practices (in some cases simplified and relaxed). If in doubt, please take a peek [here](https://medium.com/@devmrin/learn-complete-gitflow-workflow-basics-how-to-from-start-to-finish-8756ad5b7394) or [here](https://nvie.com/posts/a-successful-git-branching-model/).

This practices must appear so "elite" or "cathedral style" at first, but they are intended to ensure quality and fluidity in our development process. At the end, and with the time, you'll see how this can help you out to pin-point any info from the entangle of branches, issues, Travis, etc.

If you really want to collaborate with MystyQ's development, please catch our (git)flow ASAP.

## Issues

It's all about issues. Every change must have a reference issue on which the dev team can debate about. Branches must be named after the the user name and the concrete issue he's working on.

So, if you need to do a change, fix something or add a new feature, then please open an issue or feature for it. Once you have an issue number to work with, go and create a branch from latest `develop` in **your own fork** and name it as `user_t#_short_description_of_issue`. For an example, take a look at [this branch](https://github.com/stdevPavelmc/MystiQ/tree/stdevPavelmc_t8_travis_integration).

## Commits

Commit comments must start with `Refs #8, ....`. In this case the `#8` refers to the number of the issue you're working on. Why? see it [here in action](https://github.com/swl-x/MystiQ/issues/8).

If you go to the above link and hover the mouse over the name, number and comments of the commit `d4a19cd`, then you'll notice that GitHub does a great job by linking all together. This is possible because we mentioned the issue in the branch name and also in the commit comment.

## Pull Requests (PR)

Pull requests are intentions to merge some code or change into the main tree. You can open a PR with your proposals at any time, the only condition is that you have pushed at least a commit for an issue.

In fact, the recommended practice is to open an issue, work on it, make your first commit, and open the PR right away. This way, changes will be picked by Travis and CI/CD will fire to tell you if your changes are good o break something.

**As a general rule, a PR must include a comment on which you mention @llamaret and state that the PR is ready for review and to be merged if accepted.**

The merge action by the repo owner (@llamaret) will automatically close the pull request and the issue just by adding a comment like this to the comment of the merge `Closing issue #8...`. GitHub will do the magic and (if Travis build is a success) close the PR and the matching issue, all in just one step.

## Travis CI

This repo has Travis CI as CI/CD engine. Please, see [https://travis-ci.org/llamaret/MystiQ](https://travis-ci.org/llamaret/MystiQ) to check the status and latest tests.

Travis CI is configured to use your email for notifications of success/fail on the build you triggers. In the near future we will implements GitHub & Travis CI notifications to the Telegram channel (pending Issue for this).

## Deploys

Automatic deploys to GitHub are in the plan for tomorrow (aka ASAP) and will cover at least builds for the following Operating Systems:

- Ubuntu Bionic (works on Debian 9/10)
- Windows

In all cases, we'll try to build for 32/64 bits architectures if possible.
