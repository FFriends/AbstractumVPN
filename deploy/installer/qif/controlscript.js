var requestToQuitFromApp = false;
var updaterCompleted = 0;
var desktopAppProcessRunning = false;
var appInstalledUninstallerPath;
var appInstalledUninstallerPath_x86;

// --- wording -----------------------------------------------------------------
//
// The wizard's language is the system locale and cannot be chosen by the user:
// Qt Installer Framework loads the translation matching it, offers no page to
// pick one, and has no command line switch for it. Its own chrome - the Next
// and Cancel buttons, the target directory and licence pages - is translated by
// the framework, which ships Russian and English both.
//
// Our own pages are not, so they read the same value and pick their strings
// here. No .qm of our own: producing one needs lrelease, and this project has no
// local Qt toolchain to run it with.
//
// Headline words stay English on purpose - DEPLOY and PURGE are graphic
// elements, not prose.

function isRussian()
{
    return String(installer.value("UILanguage")).toLowerCase().indexOf("ru") === 0;
}

var STRINGS = {
    ru: {
        welcomeContext:  "[ УСТАНОВКА · %1 ]",
        welcomeBody:     "VPN-клиент, который сам поднимает сервер. Даёте машину — он ставит " +
                         "туда VPN по SSH и подключается.\n\nБез аккаунтов. Без подписок. Без посредника.",
        welcomeFacts:    "> СЛУЖБА   требует прав администратора\n> НА ДИСКЕ  около 214 МБ",
        doneHeadline:    "Развёрнуто",
        doneDetail:      "СЛУЖБА ЗАРЕГИСТРИРОВАНА И ЗАПУЩЕНА",
        failHeadline:    "Установка прервана",
        failDetail:      "Изменения отменены. Подробности — в журнале установки.",
        runApp:          "Запустить AbstractumVPN"
    },
    en: {
        welcomeContext:  "[ SETUP · %1 ]",
        welcomeBody:     "A VPN client that builds the server for you. You give it a machine, " +
                         "it installs the VPN there over SSH and connects.\n\nNo accounts. " +
                         "No subscriptions. No operator in the middle.",
        welcomeFacts:    "> SERVICE  requires administrator rights\n> ON DISK  about 214 MB",
        doneHeadline:    "Deployed",
        doneDetail:      "SERVICE REGISTERED AND RUNNING",
        failHeadline:    "Installation aborted",
        failDetail:      "Changes were rolled back. See the installation log for details.",
        runApp:          "Run AbstractumVPN"
    }
};

function tr(key)
{
    return STRINGS[isRussian() ? "ru" : "en"][key];
}

function platformTag()
{
    if (runningOnWindows()) {
        return "WINDOWS X64";
    }
    if (runningOnLinux()) {
        return "LINUX X64";
    }
    return "MACOS";
}

function appName()
{
    return installer.value("Name");
}

function appExecutableFileName()
{
    if (runningOnWindows()) {
        return appName() + ".exe";
    } else {
        return appName();
    }
}

function appInstalled()
{
    if (runningOnWindows()) {
        appInstalledUninstallerPath = installer.value("RootDir") + "Program Files/AbstractumVPN/maintenancetool.exe";
        appInstalledUninstallerPath_x86 = installer.value("RootDir") + "Program Files (x86)/AbstractumVPN/maintenancetool.exe";
    } else if (runningOnMacOS()){
        appInstalledUninstallerPath = "/Applications/" + appName() + ".app/maintenancetool.app/Contents/MacOS/maintenancetool";
    } else if (runningOnLinux()){
        appInstalledUninstallerPath = "/opt/" + appName() + "/maintenancetool";
    }

    return installer.fileExists(appInstalledUninstallerPath) || installer.fileExists(appInstalledUninstallerPath_x86);
}

function endsWith(str, suffix)
{
    return str.indexOf(suffix, str.length - suffix.length) !== -1;
}

function runningOnWindows()
{
    return (installer.value("os") === "win");
}

function runningOnMacOS()
{
    return (installer.value("os") === "mac");
}

function runningOnLinux()
{
    return ((installer.value("os") === "linux") || (installer.value("os") === "x11"));
}

function sleep(milliseconds) {
    var currentTime = new Date().getTime();
    while (currentTime + milliseconds >= new Date().getTime()) {}
}

function raiseInstallerWindow()
{
    if (!runningOnMacOS()) {
        return;
    }

    var result = installer.execute("/bin/bash", ["-c", "ps -A | grep -m1 '" + appName() + "' | awk '{print $1}'"]);
    if (Number(result[0]) > 0) {
        var arg = 'tell application \"System Events\" ' +
                '\n      set frontmost of the first process whose unix id is ' + Number(result[0]) + ' to true ' +
                '\n      end tell' +
                '\n       ';
        installer.execute("osascript", ["-e", arg]);
    }
}

function appProcessIsRunning()
{
    if (runningOnWindows()) {
        var result = installer.execute("tasklist");
        if ( Number(result[1]) === 0 ) {
            if (result[0].indexOf(appExecutableFileName()) !== -1) {
                return true;
            }
        }
    } else {
        return checkProcessIsRunning("pgrep -x '" + appName() + "'")
    }

    return false;
}

function checkProcessIsRunning(arg)
{
    var cmdArgs = ["-c", arg];
    var result = installer.execute("/bin/bash", cmdArgs);
    var lines = result[0].trim().split(/\n+/);
    var resultArg1 = Number(lines[0])
    if (resultArg1 >= 2) {
        return true;
    }
    return false;
}

function requestToQuit(installer,gui)
{
    requestToQuitFromApp = true;

    installer.setDefaultPageVisible(QInstaller.IntroductionPage, false);
    installer.setDefaultPageVisible(QInstaller.TargetDirectory, false);
    installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
    installer.setDefaultPageVisible(QInstaller.LicenseCheck, false);
    installer.setDefaultPageVisible(QInstaller.StartMenuSelection, false);
    installer.setDefaultPageVisible(QInstaller.ReadyForInstallation, false);
    installer.setDefaultPageVisible(QInstaller.PerformInstallation, false);
    installer.setDefaultPageVisible(QInstaller.FinishedPage, false);

    gui.clickButton(buttons.NextButton);
    gui.clickButton(buttons.FinishButton);
    gui.clickButton(buttons.CancelButton);

    if (runningOnWindows()) {
        installer.setCancelled();
    }
}


Controller.prototype.PerformInstallationPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.LicenseAgreementPageCallback = function()
{
    // Only stepped over when updating: the licence has not changed between two
    // builds of the same product, and an update that stops to ask about it is
    // an update people cancel. A fresh install shows it and waits.
    if (installer.isUpdater()) {
        gui.clickButton(buttons.NextButton);
    }
}

Controller.prototype.FinishedPageCallback = function ()
{
    if (desktopAppProcessRunning) {
        gui.clickButton(buttons.FinishButton);
    } else if (installer.isUpdater()) {
        installer.autoAcceptMessageBoxes();
        gui.clickButton(buttons.FinishButton);
    }
}

Controller.prototype.RestartPageCallback = function ()
{
    updaterCompleted = 1;
    gui.clickButton(buttons.FinishButton);
}

Controller.prototype.StartMenuDirectoryPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ComponentSelectionPageCallback = function()
{
    gui.clickButton(buttons.NextButton);
}

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    if (installer.isUpdater()) {
        gui.clickButton(buttons.CommitButton);
    }
}

Controller.prototype.TargetDirectoryPageCallback = function ()
{
    var widget = gui.pageById(QInstaller.TargetDirectory);

    if (widget !== null) {
        widget.BrowseDirectoryButton.clicked.disconnect(onBrowseButtonClicked);
        widget.BrowseDirectoryButton.clicked.connect(onBrowseButtonClicked);

        // Stepped over only when updating - the path is already decided and
        // must not change under an installed service.
        if (installer.isUpdater()) {
            gui.clickButton(buttons.NextButton);
        }
    }
}

// Our own pages replace the framework's introduction and finished screens.
// They are shipped through USER_INTERFACES in cmake/CPack.cmake and inserted
// here; the built-in ones are hidden so the two do not both appear.
//
// If a page fails to load - a typo in the .ui, a file missing from the package -
// findChild returns null and this quietly does nothing rather than throwing,
// which would abort the installer. The built-in page stays visible in that case,
// so the worst outcome is an unstyled wizard, not one that cannot install.
function installCustomPages()
{
    var welcome = gui.pageWidgetByObjectName("DynamicWelcomePage");
    if (welcome !== null) {
        installer.setDefaultPageVisible(QInstaller.Introduction, false);

        var context = welcome.findChild("ContextLabel");
        if (context !== null) {
            context.setText(tr("welcomeContext").replace("%1", platformTag()));
        }
        var body = welcome.findChild("BodyLabel");
        if (body !== null) {
            body.setText(tr("welcomeBody"));
        }
        var facts = welcome.findChild("FactsLabel");
        if (facts !== null) {
            facts.setText(tr("welcomeFacts"));
        }
    }

}

// The finished page is deliberately still the framework's own. It carries the
// Finish button and the hand-off that launches the application, and that flow
// works today; replacing it blind - with no local build to try it on - risks
// ending an otherwise good installation with an app that does not start. The
// stylesheet still applies to it, so it is dark and on-brand, just plainer.
// Revisit once the first release build proves the custom page mechanism.

Controller.prototype.DynamicWelcomePageCallback = function ()
{
    installCustomPages();
}

Controller.prototype.IntroductionPageCallback = function ()
{
    var widget = gui.currentPageWidget();
    if (installer.isUpdater() && updaterCompleted === 1) {
        gui.clickButton(buttons.FinishButton);
        gui.clickButton(buttons.CancelButton);
        return;
    }

    if (installer.isUninstaller()) {
        if (widget !== null) {
            widget.findChild("PackageManagerRadioButton").visible = false;
            widget.findChild("UpdaterRadioButton").visible = false;
        }
    }

    if (installer.isUpdater()) {
        gui.clickButton(buttons.NextButton);
    }
}

onBrowseButtonClicked = function()
{
    var widget = gui.pageById(QInstaller.TargetDirectory);
    if (widget !== null) {
        if (runningOnWindows()) {
            // On Windows we are appending \<APP_NAME> if selected path don't ends with <APP_NAME>
            var targetDir = widget.TargetDirectoryLineEdit.text;
            if (! endsWith(targetDir, appName())) {
                targetDir = targetDir + "\\" + appName();
            }
            installer.setValue("TargetDir", targetDir);
            widget.TargetDirectoryLineEdit.setText(installer.value("TargetDir"));
        }
    }
}

onNextButtonClicked = function()
{
    var widget = gui.pageById(QInstaller.TargetDirectory);
    if (widget !== null) {
        installer.setValue("APP_BUNDLE_TARGET_DIR", widget.TargetDirectoryLineEdit.text);
    }
}

function Controller () {
    console.log("OS: %1, architecture: %2".arg(systemInfo.prettyProductName).arg(systemInfo.currentCpuArchitecture));

    if (installer.isInstaller() || installer.isUpdater()) {
        console.log("Check if app already installed: " + appInstalled());
    }

    if (runningOnWindows()) {
        installer.setValue("AllUsers", "true");
    }

    if (installer.isInstaller()) {
        // Component selection and the start menu page stay hidden - there is one
        // component and one shortcut, so both would be a page with nothing to
        // decide on. The target directory and the licence are shown again as of
        // 07.08.2026: people installing a VPN tend to want to know where it goes
        // and under what licence.
        installer.setDefaultPageVisible(QInstaller.ComponentSelection, false);
        installer.setDefaultPageVisible(QInstaller.StartMenuDirectoryPage, false);

        isDesktopAppProcessRunningMessageLoop();

        if (requestToQuitFromApp === true) {
            requestToQuit(installer, gui);
            return;
        }

        if (runningOnMacOS()) {
            installer.setMessageBoxAutomaticAnswer("OverwriteTargetDirectory", QMessageBox.Yes);
        }

        if (appInstalled()) {
            if (QMessageBox.Ok === QMessageBox.information("os.information", appName(),
                                                           qsTr("The application is already installed.") + " " +
                                                           qsTr("We need to remove the old installation first. Do you wish to proceed?"),
                                                           QMessageBox.Ok | QMessageBox.Cancel)) {


                if (appInstalled()) {
                    var resultArray = [];

                    if (installer.fileExists(appInstalledUninstallerPath_x86)) {
                        console.log("Starting uninstallation " + appInstalledUninstallerPath_x86);
                        resultArray = installer.execute(appInstalledUninstallerPath_x86);
                    }

                    if (installer.fileExists(appInstalledUninstallerPath)) {
                        console.log("Starting uninstallation " + appInstalledUninstallerPath);
                        resultArray = installer.execute(appInstalledUninstallerPath);
                    }

                    console.log("Uninstaller finished with code: " + resultArray[1])

                    if (Number(resultArray[1]) !== 0) {
                        console.log("Uninstallation aborted by user");
                        installer.setCancelled();
                        return;
                    } else {
                        for (var i = 0; i < 300; i++) {
                            sleep(100);
                            if (!installer.fileExists(appInstalledUninstallerPath)) {
                                break;
                            }
                        }
                    }
                }

                raiseInstallerWindow();

            } else {
                console.log("Request to quit from user");
                installer.setCancelled();
                return;
            }
        }

    } else if (installer.isUninstaller()) {
        isDesktopAppProcessRunningMessageLoop();

        if (requestToQuitFromApp === true) {
            requestToQuit(installer, gui);
            return;
        }

    } else if (installer.isUpdater()) {
        installer.setMessageBoxAutomaticAnswer("cancelInstallation", QMessageBox.No);
        installer.installationFinished.connect(function() {
            gui.clickButton(buttons.NextButton);
        });
    }
}

isDesktopAppProcessRunningMessageLoop = function ()
{
    if (requestToQuitFromApp === true) {
        return;
    }

    if (installer.isUpdater()) {
        for (var i = 0; i < 400; i++) {
            desktopAppProcessRunning = appProcessIsRunning();
            if (!desktopAppProcessRunning) {
                break;
            }
        }
    }
    desktopAppProcessRunning = appProcessIsRunning();

    if (desktopAppProcessRunning) {
        var result = QMessageBox.warning("QMessageBox", appName() + " installer",
                                         appName() + " is active. Close the app and press \"Retry\" button to continue installation. Press \"Abort\" button to abort the installer and exit.",
                                         QMessageBox.Retry | QMessageBox.Abort);
        if (result === QMessageBox.Retry) {
            isDesktopAppProcessRunningMessageLoop();
        } else {
            requestToQuitFromApp = true;
            return;
        }
    }
}
