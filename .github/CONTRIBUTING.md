## Before reporting a bug
* Update to the latest *development version* (**caution:** Use a clean profile or make a backup of your working profile. It may not be possible to downgrade without data loss!).
* Check if the problem persists (it may happen that it has been fixed).
* Search the *issues* for an existing bug report for your problem.

## Effective bug reporting
Using the following tips will **greatly** increase the chances of your issue being '''noticed''' and '''fixed''' quickly.

* Have a well-written bug **Summary** field – describe the issue clearly, in simple words.
* **Description** field – provide step-by-step instructions so we can reproduce the issue (*We can't fix what we can't reproduce*).
* Attach screenshots if possible.
* Attach your [VersionInfo](https://wiki.miranda-ng.org/index.php?title=Version_information).
* Attach a [crash report](https://wiki.miranda-ng.org/index.php?title=Crash_reports) if you are experiencing crashes (It includes VersionInfo as well).
* Attach a [network log](https://wiki.miranda-ng.org/index.php?title=Network_log) captured if you are experiencing connectivity issues.
* If Miranda freezes, hangs up or consumes CPU, attach [information obtained by Process Explorer tool](https://wiki.miranda-ng.org/index.php?title=Using_Process_Explorer_as_the_debugging_tool).

### How to attach files
* Attach files by dragging & dropping, selecting them, or pasting from the clipboard
* Press *Submit new issue*.

## I know when it got broken
If you know things got broken in some version or at some particular time, then it's good because you can try different releases from different revisions and narrow down where exactly the problem started to happen.
* you can either compile the revisions yourself (that requires more knowledge and time)
* or ask someone in our [jabber conference](xmpp:miranda-ng-int@conference.jabber.ru?join) to help you identify potentially problematic revisions and then ask to compile those revisions for you.

Then you can try to reproduce your problem on each of them and find out when exactly it got broken. With that info it's easy to reach out the developer who commited the broken commit and ask him to look at it.

## After reporting an issue
* Monitor the issues you report.
* Provide feedback and additional information if requested.
* Don't argue about whether something is a bug or not.
