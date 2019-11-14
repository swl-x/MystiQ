# Developer process & work flows

This document stablish the process and workflows as per github and gitflow good practices (in some cases simplified and relaxed), if in doubt please take a peek [here](https://medium.com/@devmrin/learn-complete-gitflow-workflow-basics-how-to-from-start-to-finish-8756ad5b7394) or [here](https://nvie.com/posts/a-successful-git-branching-model/)

This practices must appear so "elite" or "cathedral style" at first, but believe me you will thanks me for teaching you this if you pretend to work for a bigger or professional soft company. At the end and with the time you will see how easy is to pin-point any info from the entangle of branches, issues, travis, etc.

At first we will be easy with this but people, please catch the (git)flow ASAP.

## Issues!

Its all about issues, every change must have a reference issue in which the dev team can debate about, and branches that name the user and issue it's working on.

So if you need to do a change, fix something or add a new feature, please open an issue or feature for it. Once you have a issue number to work with, create a branch from latest `develop` in YOUR own fork and name it `user_t#_short_description_of_issue` see [here](https://github.com/stdevPavelmc/MystiQ/tree/stdevPavelmc_t8_travis_integration) where I created a branch named `stdevPavelmc_t8_travis_integration`

## Commits!

All commits comments must start with `Refs #8, ....` where in this case the #8 refers to the issue you are working on, why? see it [here in action](https://github.com/llamaret/MystiQ/issues/8)

Hover the mouse over the name, number and comments of the commit `d4a19cd` github does a great job by linking all together, this is possible because we mentiones the issue in the branch name and also in the commit comment.

## Travis!

WIP