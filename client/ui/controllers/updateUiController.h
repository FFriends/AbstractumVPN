#ifndef UPDATEUICONTROLLER_H
#define UPDATEUICONTROLLER_H

#include <QObject>

#include "core/controllers/updateController.h"

class UpdateUiController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString changelogText READ getChangelogText NOTIFY updateFound)
    Q_PROPERTY(QString headerText READ getHeaderText NOTIFY updateFound)

public:
    explicit UpdateUiController(UpdateController* updateController, QObject *parent = nullptr);

    QString getHeaderText() const;
    QString getChangelogText() const;
    QString getVersion() const;

public slots:
    void checkForUpdates();
    void runInstaller();

signals:
    void updateFound();
    void updateNotFound();

private:
    UpdateController* m_updateController;

    // The check that runs by itself at startup must stay silent. Only a check
    // the user asked for is allowed to answer "nothing new", otherwise every
    // launch would greet them with a notification.
    bool m_manualCheck = false;
};

#endif // UPDATEUICONTROLLER_H
