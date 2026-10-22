#pragma once

#include <QAbstractListModel>
#include <QHash>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>

class Library;

class GameModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        GameIdRole = Qt::UserRole + 1,
        TitleRole,
        SystemIdRole,
        SystemNameRole,
        ShortNameRole,
        AccentRole,
        YearRole,
        DeveloperRole,
        BlurbRole,
        PathRole,
        FileReadyRole,
        PlayableRole,
        FavoriteRole,
        ExampleRole,
        KindRole,
        LastPlayedRole,
        BlockRole
    };

    explicit GameModel(Library *library);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void refresh();

private:
    Library *m_library;
};

class SystemModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        SystemIdRole = Qt::UserRole + 1,
        NameRole,
        ShortNameRole,
        YearRole,
        ManufacturerRole,
        BlurbRole,
        AccentRole,
        SecondaryRole,
        ArtRole,
        ExtensionsRole,
        EmulatorReadyRole,
        EmulatorLabelRole,
        GameCountRole,
        ReadyCountRole,
        BuiltinRole,
        BiosNoteRole,
        BiosRequiredRole,
        BiosReadyRole,
        FlatpakIdRole
    };

    explicit SystemModel(Library *library);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void refresh();
    void touch();

private:
    Library *m_library;
};

class Library : public QObject {
    Q_OBJECT
    Q_PROPERTY(GameModel *games READ games CONSTANT)
    Q_PROPERTY(SystemModel *systems READ systems CONSTANT)
    Q_PROPERTY(int revision READ revision NOTIFY revisionChanged)
    Q_PROPERTY(int systemCount READ systemCount NOTIFY revisionChanged)
    Q_PROPERTY(int listedCount READ listedCount NOTIFY revisionChanged)
    Q_PROPERTY(int totalGames READ totalGames NOTIFY revisionChanged)
    Q_PROPERTY(QString dataDir READ dataDir CONSTANT)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QString systemFilter READ systemFilter WRITE setSystemFilter NOTIFY systemFilterChanged)
    Q_PROPERTY(QString listMode READ listMode WRITE setListMode NOTIFY listModeChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(QString busyLog READ busyLog NOTIFY busyLogChanged)

public:
    explicit Library(QObject *parent = nullptr);

    GameModel *games() const { return m_gameModel; }
    SystemModel *systems() const { return m_systemModel; }
    int revision() const { return m_revision; }
    int systemCount() const { return m_systems.size(); }
    int listedCount() const { return m_visible.size(); }
    int totalGames() const { return m_games.size(); }
    QString dataDir() const { return m_dataDir; }
    QString status() const { return m_status; }
    QString query() const { return m_query; }
    QString systemFilter() const { return m_systemFilter; }
    QString listMode() const { return m_listMode; }
    bool busy() const { return m_busy; }
    QString busyLog() const { return m_busyLog; }

    void setQuery(const QString &value);
    void setSystemFilter(const QString &value);
    void setListMode(const QString &value);

    Q_INVOKABLE QVariantMap game(const QString &id) const;
    Q_INVOKABLE QVariantMap system(const QString &id) const;
    Q_INVOKABLE QString systemIdAt(int row) const;
    Q_INVOKABLE bool systemReady(const QString &id) const;
    Q_INVOKABLE QVariantList playableGames() const;
    Q_INVOKABLE QVariantList classicGames() const;
    Q_INVOKABLE QString commandForGame(const QString &id) const;
    Q_INVOKABLE QString commandForSystem(const QString &id) const;
    Q_INVOKABLE QString localPath(const QVariant &value) const;

    Q_INVOKABLE void addFiles(const QString &systemId, const QVariantList &urls);
    Q_INVOKABLE void scanDirectory(const QString &systemId, const QVariant &folder, bool recursive);
    Q_INVOKABLE void launchGame(const QString &id);
    Q_INVOKABLE void launchSystem(const QString &id);
    Q_INVOKABLE void removeGame(const QString &id);
    Q_INVOKABLE void toggleFavorite(const QString &id);
    Q_INVOKABLE void setTitle(const QString &id, const QString &title);
    Q_INVOKABLE void setGamePath(const QString &id, const QVariant &path);
    Q_INVOKABLE void updateSystem(const QString &id, const QString &command, const QString &gameArgs,
                                  const QString &plainArgs, const QString &bios, const QString &extensions);
    Q_INVOKABLE void addSystem(const QString &name, const QString &shortName, const QString &extensions,
                               const QString &command, const QString &accent);
    Q_INVOKABLE void removeSystem(const QString &id);
    Q_INVOKABLE void installFlatpak(const QString &id);
    Q_INVOKABLE void probe();
    Q_INVOKABLE void openDataFolder() const;
    Q_INVOKABLE void revealPath(const QString &path) const;
    Q_INVOKABLE void copyText(const QString &text);
    Q_INVOKABLE void clearStatus();
    Q_INVOKABLE void restoreExamples();

    int selfCheck() const;
    void writeDesktopEntry() const;

    struct System {
        QString id;
        QString name;
        QString shortName;
        QString year;
        QString manufacturer;
        QString blurb;
        QString accent;
        QString secondary;
        QString art;
        QStringList extensions;
        QStringList candidates;
        QStringList defaultPlainArgs;
        QStringList defaultGameArgs;
        QHash<QString, QStringList> argsByExt;
        QString biosFlag;
        QString biosNote;
        bool biosRequired = false;
        QString flatpakId;
        QString installHint;
        bool builtin = true;
        QString commandOverride;
        QString plainOverride;
        QString gameOverride;
        QString extensionsOverride;
        QString biosPath;
        bool biosUserSet = false;
        bool emulatorReady = false;
        QString resolvedDisplay;
        QStringList resolvedCommand;
    };

    struct Game {
        QString id;
        QString systemId;
        QString title;
        QString year;
        QString developer;
        QString blurb;
        QString path;
        QString kind;
        QString bundled;
        QString pathHint;
        QString lastPlayed;
        bool favorite = false;
        bool example = false;
    };

signals:
    void statusChanged();
    void revisionChanged();
    void queryChanged();
    void systemFilterChanged();
    void listModeChanged();
    void busyChanged();
    void busyLogChanged();

private:
    friend class GameModel;
    friend class SystemModel;

    System *findSystem(const QString &id);
    const System *findSystem(const QString &id) const;
    Game *findGame(const QString &id);
    const Game *findGame(const QString &id) const;
    int systemRow(const QString &id) const;

    void load();
    void save() const;
    void applyCatalog(const QJsonObject &catalog, const QJsonObject &saved);
    void bindLocalFiles();
    void probeSystems();
    QString findBios(const System &system) const;
    void rebuildVisible();
    void notifyChanged();
    void setStatus(const QString &text);
    void setBusy(bool busy);

    QStringList effectiveExtensions(const System &system) const;
    QString findProgram(const QString &name) const;
    QStringList resolveCommand(const System &system, QString *error) const;
    QStringList launchArguments(const System &system, const QString &filePath) const;
    QStringList fullCommand(const System &system, const QString &filePath, QString *error) const;
    bool startCommand(const QStringList &command, const QString &workDir, QString *error) const;
    QString systemForFile(const QString &path) const;
    bool extensionAllowed(const System &system, const QString &path) const;
    int ingestFile(const QString &systemId, const QString &path, bool forceSystem, int *linked, int *skipped);
    QString expandTemplate(QString text, const QString &filePath, const System &system) const;
    QString extractDemo(const QString &name);
    int countGames(const QString &systemId, bool playableOnly) const;
    bool maybeLinkExample(const QString &systemId, const QString &path);
    QVariantMap gameMap(const Game &game) const;
    QVariantMap systemMap(const System &system) const;
    bool fileReady(const Game &game) const;
    bool playable(const Game &game) const;
    QString blockReason(const Game &game) const;
    QString prettyPlayed(const QString &iso) const;

    GameModel *m_gameModel = nullptr;
    SystemModel *m_systemModel = nullptr;
    QList<System> m_systems;
    QList<Game> m_games;
    QList<int> m_visible;
    QStringList m_hidden;
    QString m_dataDir;
    QString m_status;
    QString m_query;
    QString m_systemFilter;
    QString m_listMode = QStringLiteral("all");
    QString m_busyLog;
    int m_revision = 0;
    bool m_busy = false;
    class QProcess *m_install = nullptr;
};
