#include "library.h"

#include <QClipboard>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QUuid>

#include <algorithm>

namespace {

QStringList jsonStrings(const QJsonValue &value)
{
    QStringList out;
    if (value.isArray()) {
        for (const QJsonValue &item : value.toArray())
            out << item.toString();
    } else if (value.isString() && !value.toString().isEmpty()) {
        out << value.toString();
    }
    return out;
}

QString quoteArg(const QString &arg)
{
    if (arg.contains(QLatin1Char(' ')) || arg.contains(QLatin1Char('"')))
        return QLatin1Char('"') + QString(arg).replace(QLatin1Char('"'), QStringLiteral("\\\"")) + QLatin1Char('"');
    return arg;
}

QString joinCommand(const QStringList &args)
{
    QStringList quoted;
    for (const QString &arg : args)
        quoted << quoteArg(arg);
    return quoted.join(QLatin1Char(' '));
}

QString normTitle(QString text)
{
    text = text.toLower();
    text.replace(QLatin1Char('&'), QStringLiteral(" and "));
    text.replace(QLatin1Char('_'), QLatin1Char(' '));
    text.replace(QLatin1Char('-'), QLatin1Char(' '));
    text.remove(QRegularExpression(QStringLiteral("\\b(19|20)\\d{2}\\b")));
    QString out;
    for (const QChar character : text) {
        if (character.isLetterOrNumber() || character.isSpace())
            out += character;
    }
    out = out.simplified();
    if (out.startsWith(QStringLiteral("the ")))
        out = out.mid(4);
    return out;
}

QString prettyTitle(QString base)
{
    base.replace(QLatin1Char('_'), QLatin1Char(' '));
    base.replace(QLatin1Char('.'), QLatin1Char(' '));
    base.remove(QRegularExpression(QStringLiteral("\\s*[\\(\\[]?(19|20)\\d{2}[\\)\\]]?\\s*$")));
    return base.simplified();
}

QString variantPath(const QVariant &value)
{
    if (value.canConvert<QUrl>()) {
        const QUrl url = value.toUrl();
        if (url.isLocalFile())
            return url.toLocalFile();
    }
    const QString text = value.toString();
    const QUrl url(text);
    if (url.isLocalFile())
        return url.toLocalFile();
    return text;
}

} // namespace

GameModel::GameModel(Library *library)
    : QAbstractListModel(library)
    , m_library(library)
{
}

int GameModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_library->m_visible.size());
}

QVariant GameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_library->m_visible.size())
        return {};
    const Library::Game &game = m_library->m_games.at(m_library->m_visible.at(index.row()));
    const Library::System *system = m_library->findSystem(game.systemId);
    switch (role) {
    case GameIdRole: return game.id;
    case TitleRole: return game.title;
    case SystemIdRole: return game.systemId;
    case SystemNameRole: return system ? system->name : QString();
    case ShortNameRole: return system ? system->shortName : QString();
    case AccentRole: return system ? system->accent : QStringLiteral("#e6a23c");
    case YearRole: return game.year;
    case DeveloperRole: return game.developer;
    case BlurbRole: return game.blurb;
    case PathRole: return game.path;
    case FileReadyRole: return m_library->fileReady(game);
    case PlayableRole: return m_library->playable(game);
    case FavoriteRole: return game.favorite;
    case ExampleRole: return game.example;
    case KindRole: return game.kind;
    case LastPlayedRole: return m_library->prettyPlayed(game.lastPlayed);
    case BlockRole: return m_library->blockReason(game);
    default: return {};
    }
}

QHash<int, QByteArray> GameModel::roleNames() const
{
    return {
        {GameIdRole, "gameId"},
        {TitleRole, "title"},
        {SystemIdRole, "systemId"},
        {SystemNameRole, "systemName"},
        {ShortNameRole, "shortName"},
        {AccentRole, "accent"},
        {YearRole, "year"},
        {DeveloperRole, "developer"},
        {BlurbRole, "blurb"},
        {PathRole, "path"},
        {FileReadyRole, "fileReady"},
        {PlayableRole, "playable"},
        {FavoriteRole, "favorite"},
        {ExampleRole, "example"},
        {KindRole, "kind"},
        {LastPlayedRole, "lastPlayed"},
        {BlockRole, "blockReason"},
    };
}

void GameModel::refresh()
{
    beginResetModel();
    endResetModel();
}

SystemModel::SystemModel(Library *library)
    : QAbstractListModel(library)
    , m_library(library)
{
}

int SystemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(m_library->m_systems.size());
}

QVariant SystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_library->m_systems.size())
        return {};
    const Library::System &system = m_library->m_systems.at(index.row());
    switch (role) {
    case SystemIdRole: return system.id;
    case NameRole: return system.name;
    case ShortNameRole: return system.shortName;
    case YearRole: return system.year;
    case ManufacturerRole: return system.manufacturer;
    case BlurbRole: return system.blurb;
    case AccentRole: return system.accent;
    case SecondaryRole: return system.secondary;
    case ArtRole: return system.art;
    case ExtensionsRole: return m_library->effectiveExtensions(system).join(QStringLiteral(" "));
    case EmulatorReadyRole: return system.emulatorReady;
    case EmulatorLabelRole: return system.emulatorReady ? QFileInfo(system.resolvedCommand.value(0)).fileName() : QStringLiteral("Not found");
    case GameCountRole: return m_library->countGames(system.id, false);
    case ReadyCountRole: return m_library->countGames(system.id, true);
    case BuiltinRole: return system.builtin;
    case BiosNoteRole: return system.biosNote;
    case BiosRequiredRole: return system.biosRequired;
    case BiosReadyRole: return !system.biosRequired || !system.biosPath.isEmpty();
    case FlatpakIdRole: return system.flatpakId;
    default: return {};
    }
}

QHash<int, QByteArray> SystemModel::roleNames() const
{
    return {
        {SystemIdRole, "systemId"},
        {NameRole, "name"},
        {ShortNameRole, "shortName"},
        {YearRole, "year"},
        {ManufacturerRole, "manufacturer"},
        {BlurbRole, "blurb"},
        {AccentRole, "accent"},
        {SecondaryRole, "secondary"},
        {ArtRole, "art"},
        {ExtensionsRole, "extensions"},
        {EmulatorReadyRole, "emulatorReady"},
        {EmulatorLabelRole, "emulatorLabel"},
        {GameCountRole, "gameCount"},
        {ReadyCountRole, "readyCount"},
        {BuiltinRole, "builtin"},
        {BiosNoteRole, "biosNote"},
        {BiosRequiredRole, "biosRequired"},
        {BiosReadyRole, "biosReady"},
        {FlatpakIdRole, "flatpakId"},
    };
}

void SystemModel::refresh()
{
    beginResetModel();
    endResetModel();
}

void SystemModel::touch()
{
    if (rowCount() < 1)
        return;
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0));
}

Library::Library(QObject *parent)
    : QObject(parent)
{
    m_dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (m_dataDir.isEmpty())
        m_dataDir = QDir::homePath() + QStringLiteral("/.local/share/retro-center");
    QDir().mkpath(m_dataDir + QStringLiteral("/demos"));
    m_gameModel = new GameModel(this);
    m_systemModel = new SystemModel(this);
    load();
}

void Library::setQuery(const QString &value)
{
    if (m_query == value)
        return;
    m_query = value;
    emit queryChanged();
    rebuildVisible();
    m_gameModel->refresh();
    ++m_revision;
    emit revisionChanged();
}

void Library::setSystemFilter(const QString &value)
{
    if (m_systemFilter == value)
        return;
    m_systemFilter = value;
    emit systemFilterChanged();
    rebuildVisible();
    m_gameModel->refresh();
    ++m_revision;
    emit revisionChanged();
}

void Library::setListMode(const QString &value)
{
    const QString next = (value == QLatin1String("ready") || value == QLatin1String("missing")
                          || value == QLatin1String("favorites") || value == QLatin1String("recent"))
        ? value
        : QStringLiteral("all");
    if (m_listMode == next)
        return;
    m_listMode = next;
    emit listModeChanged();
    rebuildVisible();
    m_gameModel->refresh();
    ++m_revision;
    emit revisionChanged();
}

void Library::setStatus(const QString &text)
{
    m_status = text;
    emit statusChanged();
}

void Library::setBusy(bool busy)
{
    if (m_busy == busy)
        return;
    m_busy = busy;
    emit busyChanged();
}

void Library::clearStatus()
{
    setStatus(QString());
}

Library::System *Library::findSystem(const QString &id)
{
    for (System &system : m_systems) {
        if (system.id == id)
            return &system;
    }
    return nullptr;
}

const Library::System *Library::findSystem(const QString &id) const
{
    for (const System &system : m_systems) {
        if (system.id == id)
            return &system;
    }
    return nullptr;
}

Library::Game *Library::findGame(const QString &id)
{
    for (Game &game : m_games) {
        if (game.id == id)
            return &game;
    }
    return nullptr;
}

const Library::Game *Library::findGame(const QString &id) const
{
    for (const Game &game : m_games) {
        if (game.id == id)
            return &game;
    }
    return nullptr;
}

int Library::systemRow(const QString &id) const
{
    for (int i = 0; i < m_systems.size(); ++i) {
        if (m_systems.at(i).id == id)
            return i;
    }
    return -1;
}

QStringList Library::effectiveExtensions(const System &system) const
{
    if (system.extensionsOverride.trimmed().isEmpty())
        return system.extensions;
    QStringList out;
    const QStringList parts = system.extensionsOverride.split(QRegularExpression(QStringLiteral("[,\\s]+")), Qt::SkipEmptyParts);
    for (QString ext : parts) {
        ext = ext.toLower();
        if (!ext.startsWith(QLatin1Char('.')))
            ext.prepend(QLatin1Char('.'));
        out << ext;
    }
    return out;
}

QString Library::findProgram(const QString &name) const
{
    if (name.isEmpty())
        return {};
    const QFileInfo info(name);
    if (info.isAbsolute())
        return info.isExecutable() ? info.absoluteFilePath() : QString();
    if (name.contains(QLatin1Char('/')))
        return info.isExecutable() ? info.absoluteFilePath() : QString();

    QStringList paths = qEnvironmentVariable("PATH").split(QLatin1Char(':'), Qt::SkipEmptyParts);
    const QString homeBin = QDir::homePath() + QStringLiteral("/.local/bin");
    if (!paths.contains(homeBin))
        paths.prepend(homeBin);
#ifdef Q_OS_WIN
    const QChar sep = QLatin1Char(';');
    paths = qEnvironmentVariable("PATH").split(sep, Qt::SkipEmptyParts);
#endif
    return QStandardPaths::findExecutable(name, paths);
}

QStringList Library::resolveCommand(const System &system, QString *error) const
{
    QStringList lines = system.candidates;
    if (!system.commandOverride.trimmed().isEmpty())
        lines.prepend(system.commandOverride.trimmed());
    for (const QString &line : lines) {
        QStringList parts = QProcess::splitCommand(line);
        if (parts.isEmpty())
            continue;
        const QString program = findProgram(parts.takeFirst());
        if (program.isEmpty())
            continue;
        parts.prepend(program);
        return parts;
    }
    if (error)
        *error = QStringLiteral("No emulator found for %1.").arg(system.name);
    return {};
}

QString Library::expandTemplate(QString text, const QString &filePath, const System &system) const
{
    const QFileInfo info(filePath);
    text.replace(QStringLiteral("{file}"), info.absoluteFilePath());
    text.replace(QStringLiteral("{dir}"), info.absolutePath());
    text.replace(QStringLiteral("{name}"), info.completeBaseName());
    text.replace(QStringLiteral("{ext}"), info.suffix());
    text.replace(QStringLiteral("{bios}"), system.biosPath);
    text.replace(QStringLiteral("{system}"), system.id);
    return text;
}

QStringList Library::launchArguments(const System &system, const QString &filePath) const
{
    QStringList args;
    if (filePath.isEmpty()) {
        args = system.plainOverride.trimmed().isEmpty() ? system.defaultPlainArgs
                                                        : QProcess::splitCommand(system.plainOverride);
    } else if (!system.gameOverride.trimmed().isEmpty()) {
        args = QProcess::splitCommand(system.gameOverride);
    } else {
        const QString ext = QLatin1Char('.') + QFileInfo(filePath).suffix().toLower();
        args = system.argsByExt.value(ext, system.defaultGameArgs);
    }
    for (QString &arg : args)
        arg = expandTemplate(arg, filePath, system);

    if (!system.biosPath.isEmpty() && !system.biosFlag.isEmpty()) {
        if (system.biosFlag.contains(QStringLiteral("{bios}"))) {
            args.prepend(QString(system.biosFlag).replace(QStringLiteral("{bios}"), system.biosPath));
        } else {
            args.prepend(system.biosPath);
            args.prepend(system.biosFlag);
        }
    }
    return args;
}

QStringList Library::fullCommand(const System &system, const QString &filePath, QString *error) const
{
    QStringList command = resolveCommand(system, error);
    if (command.isEmpty())
        return {};

    if (QFileInfo(command.first()).fileName() == QLatin1String("flatpak")) {
        const int insertAt = (command.size() > 1 && command.at(1) == QLatin1String("run")) ? 2 : 1;
        QStringList dirs;
        const QStringList paths = {filePath, system.biosPath};
        for (const QString &path : paths) {
            if (path.isEmpty())
                continue;
            const QFileInfo info(path);
            const QString dir = info.isDir() ? info.absoluteFilePath() : info.absolutePath();
            if (!dir.isEmpty() && !dirs.contains(dir))
                dirs << dir;
        }
        for (int i = dirs.size() - 1; i >= 0; --i)
            command.insert(insertAt, QStringLiteral("--filesystem=") + dirs.at(i));
    }
    command.append(launchArguments(system, filePath));
    return command;
}

bool Library::startCommand(const QStringList &command, const QString &workDir, QString *error) const
{
    if (command.isEmpty()) {
        if (error)
            *error = QStringLiteral("There is no command to run.");
        return false;
    }
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString homeBin = QDir::homePath() + QStringLiteral("/.local/bin");
    QString path = env.value(QStringLiteral("PATH"));
#ifdef Q_OS_WIN
    const QChar sep = QLatin1Char(';');
#else
    const QChar sep = QLatin1Char(':');
#endif
    if (!path.split(sep, Qt::SkipEmptyParts).contains(homeBin))
        env.insert(QStringLiteral("PATH"), homeBin + sep + path);
    if (command.join(QLatin1Char(' ')).contains(QStringLiteral("linapple"))) {
        const QString lib = QDir::homePath() + QStringLiteral("/.local/opt/linapple/lib");
        const QString old = env.value(QStringLiteral("LD_LIBRARY_PATH"));
        env.insert(QStringLiteral("LD_LIBRARY_PATH"), old.isEmpty() ? lib : lib + QLatin1Char(':') + old);
    }

    QProcess process;
    process.setProgram(command.first());
    process.setArguments(command.mid(1));
    process.setProcessEnvironment(env);
    process.setWorkingDirectory(workDir.isEmpty() ? QDir::homePath() : workDir);
    qint64 pid = 0;
    QFile log(m_dataDir + QStringLiteral("/launch.log"));
    if (log.open(QIODevice::WriteOnly | QIODevice::Append)) {
        log.write(QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8());
        log.write("  ");
        log.write(joinCommand(command).toUtf8());
        log.write("\n");
    }
    if (!process.startDetached(&pid)) {
        if (error)
            *error = QStringLiteral("Could not start %1.").arg(command.first());
        return false;
    }
    return true;
}

bool Library::fileReady(const Game &game) const
{
    return !game.path.isEmpty() && QFileInfo::exists(game.path);
}

bool Library::playable(const Game &game) const
{
    const System *system = findSystem(game.systemId);
    return system && system->emulatorReady && fileReady(game);
}

QString Library::blockReason(const Game &game) const
{
    const System *system = findSystem(game.systemId);
    if (!system)
        return QStringLiteral("That machine was removed.");
    if (!system->emulatorReady)
        return QStringLiteral("No emulator is set for %1.").arg(system->name);
    if (game.path.isEmpty())
        return QStringLiteral("Shelf card only. Choose a disk image you own.");
    if (!QFileInfo::exists(game.path))
        return QStringLiteral("That disk image is no longer there.");
    if (system->biosRequired && system->biosPath.isEmpty())
        return QStringLiteral("No Kickstart ROM is set. FS-UAE may not boot until you add one in Settings.");
    return {};
}

QString Library::prettyPlayed(const QString &iso) const
{
    if (iso.isEmpty())
        return {};
    const QDateTime when = QDateTime::fromString(iso, Qt::ISODate);
    if (!when.isValid())
        return iso;
    return when.toString(QStringLiteral("d MMM yyyy, h:mm ap"));
}

int Library::countGames(const QString &systemId, bool playableOnly) const
{
    int count = 0;
    for (const Game &game : m_games) {
        if (game.systemId != systemId)
            continue;
        if (playableOnly && !playable(game))
            continue;
        ++count;
    }
    return count;
}

QString Library::systemForFile(const QString &path) const
{
    const QString ext = QLatin1Char('.') + QFileInfo(path).suffix().toLower();
    QStringList hits;
    for (const System &system : m_systems) {
        const QStringList extensions = effectiveExtensions(system);
        if (extensions.contains(QStringLiteral(".*")) || extensions.contains(ext))
            hits << system.id;
    }
    if (hits.isEmpty())
        return {};
    if (!m_systemFilter.isEmpty() && hits.contains(m_systemFilter))
        return m_systemFilter;
    if (hits.size() == 1)
        return hits.first();
    if (ext == QLatin1String(".dsk") && hits.contains(QStringLiteral("apple2")))
        return QStringLiteral("apple2");
    return hits.first();
}

bool Library::extensionAllowed(const System &system, const QString &path) const
{
    const QString ext = QLatin1Char('.') + QFileInfo(path).suffix().toLower();
    return effectiveExtensions(system).contains(ext);
}

bool Library::maybeLinkExample(const QString &systemId, const QString &path)
{
    const QString base = normTitle(prettyTitle(QFileInfo(path).completeBaseName()));
    if (base.isEmpty())
        return false;
    for (Game &game : m_games) {
        if (game.systemId != systemId || !game.example || game.kind == QLatin1String("bundled"))
            continue;
        const QString title = normTitle(game.title);
        const bool match = title == base || (title.size() >= 8 && base.contains(title));
        if (!match)
            continue;
        if (!game.path.isEmpty() && QFileInfo::exists(game.path)) {
            if (QFileInfo(game.path).absoluteFilePath() == QFileInfo(path).absoluteFilePath())
                return true;
            continue;
        }
        game.path = QFileInfo(path).absoluteFilePath();
        return true;
    }
    return false;
}

int Library::ingestFile(const QString &systemId, const QString &path, bool forceSystem, int *linked, int *skipped)
{
    const QFileInfo info(path);
    if (!info.exists() || !info.isFile())
        return 0;
    const QString absolute = info.absoluteFilePath();
    for (const Game &game : m_games) {
        if (!game.path.isEmpty() && QFileInfo(game.path).absoluteFilePath() == absolute) {
            if (skipped)
                ++(*skipped);
            return 0;
        }
    }

    QString chosen = systemId;
    if (!forceSystem || chosen.isEmpty()) {
        const QString detected = systemForFile(absolute);
        if (!detected.isEmpty())
            chosen = detected;
    }
    if (chosen.isEmpty() || !findSystem(chosen))
        return -1;
    if (maybeLinkExample(chosen, absolute)) {
        if (linked)
            ++(*linked);
        return 2;
    }

    Game game;
    game.id = QStringLiteral("game-") + QUuid::createUuid().toString(QUuid::Id128);
    game.systemId = chosen;
    game.title = prettyTitle(info.completeBaseName());
    if (game.title.isEmpty())
        game.title = info.fileName();
    game.path = absolute;
    game.kind = QStringLiteral("user");
    game.example = false;
    m_games.append(game);
    return 1;
}

QString Library::extractDemo(const QString &name)
{
    QFile in(QStringLiteral(":/data/demos/") + name);
    if (!in.open(QIODevice::ReadOnly))
        return {};
    const QByteArray bytes = in.readAll();
    const QString dest = m_dataDir + QStringLiteral("/demos/") + name;
    const QFileInfo current(dest);
    if (!current.exists() || current.size() != bytes.size()) {
        QDir().mkpath(m_dataDir + QStringLiteral("/demos"));
        QFile out(dest);
        if (!out.open(QIODevice::WriteOnly))
            return {};
        out.write(bytes);
    }
    return dest;
}

QString Library::findBios(const System &system) const
{
    const QString home = QDir::homePath();
    if (system.id == QLatin1String("amiga")) {
        const QStringList roots = {
            home + QStringLiteral("/FS-UAE/Kickstarts"),
            home + QStringLiteral("/Documents/FS-UAE/Kickstarts"),
            home + QStringLiteral("/.local/share/fs-uae/Kickstarts"),
            home + QStringLiteral("/.local/share/FS-UAE/Kickstarts"),
            m_dataDir + QStringLiteral("/bios"),
        };
        QString fallback;
        for (const QString &root : roots) {
            QDir dir(root);
            if (!dir.exists())
                continue;
            const QFileInfoList files = dir.entryInfoList(QDir::Files);
            for (const QFileInfo &info : files) {
                const QString lower = info.fileName().toLower();
                if (info.size() < 64 * 1024)
                    continue;
                if (lower.contains(QLatin1String("a500")) || lower.contains(QLatin1String("kick34005"))
                    || lower == QLatin1String("kick13.rom"))
                    return info.absoluteFilePath();
                if (fallback.isEmpty() && (lower.contains(QLatin1String("kick")) || lower.contains(QLatin1String("aros"))))
                    fallback = info.absoluteFilePath();
            }
        }
        return fallback;
    }
    if (system.id == QLatin1String("atarist")) {
        const QStringList files = {
            QStringLiteral("/usr/share/hatari/tos.img"),
            QStringLiteral("/usr/share/emutos/etos256us.img"),
            QStringLiteral("/usr/share/emutos/etos512us.img"),
            home + QStringLiteral("/.hatari/tos.img"),
            m_dataDir + QStringLiteral("/bios/tos.img"),
            m_dataDir + QStringLiteral("/bios/etos256us.img"),
        };
        for (const QString &file : files) {
            if (QFileInfo::exists(file))
                return file;
        }
    }
    return {};
}

void Library::bindLocalFiles()
{
    const QString home = QDir::homePath();
    for (Game &game : m_games) {
        if (!game.bundled.isEmpty()) {
            const QString dest = extractDemo(game.bundled);
            if (!dest.isEmpty() && (game.path.isEmpty() || QFileInfo(game.path).fileName() == game.bundled))
                game.path = dest;
        }
        if (game.path.isEmpty() && !game.pathHint.isEmpty()) {
            const QString hint = QString(game.pathHint).replace(QStringLiteral("${HOME}"), home);
            if (QFileInfo::exists(hint))
                game.path = hint;
        }
    }
}

void Library::probeSystems()
{
    for (System &system : m_systems) {
        if (!system.biosUserSet && system.biosPath.isEmpty())
            system.biosPath = findBios(system);
        QString error;
        system.resolvedCommand = resolveCommand(system, &error);
        system.emulatorReady = !system.resolvedCommand.isEmpty();
        system.resolvedDisplay = system.emulatorReady ? joinCommand(system.resolvedCommand) : QString();
    }
}

void Library::rebuildVisible()
{
    m_visible.clear();
    const QStringList words = m_query.toLower().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    struct Row {
        int index;
        QString key;
    };
    QList<Row> rows;
    for (int i = 0; i < m_games.size(); ++i) {
        const Game &game = m_games.at(i);
        if (!m_systemFilter.isEmpty() && game.systemId != m_systemFilter)
            continue;
        if (m_listMode == QLatin1String("ready") && !playable(game))
            continue;
        if (m_listMode == QLatin1String("missing") && fileReady(game))
            continue;
        if (m_listMode == QLatin1String("favorites") && !game.favorite)
            continue;
        if (m_listMode == QLatin1String("recent") && game.lastPlayed.isEmpty())
            continue;
        if (!words.isEmpty()) {
            const System *system = findSystem(game.systemId);
            const QString hay = (game.title + QLatin1Char(' ') + game.developer + QLatin1Char(' ') + game.year
                                 + QLatin1Char(' ') + (system ? system->name : QString())
                                 + QLatin1Char(' ') + (system ? system->shortName : QString()))
                                    .toLower();
            bool match = true;
            for (const QString &word : words) {
                if (!hay.contains(word)) {
                    match = false;
                    break;
                }
            }
            if (!match)
                continue;
        }
        rows.append({i, game.lastPlayed});
    }
    if (m_listMode == QLatin1String("recent")) {
        std::sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) { return a.key > b.key; });
    }
    for (const Row &row : rows)
        m_visible.append(row.index);
}

void Library::notifyChanged()
{
    rebuildVisible();
    m_gameModel->refresh();
    m_systemModel->touch();
    ++m_revision;
    emit revisionChanged();
}

namespace {

Library::System systemFromJson(const QJsonObject &object, bool builtin)
{
    Library::System system;
    system.id = object.value(QStringLiteral("id")).toString();
    system.name = object.value(QStringLiteral("name")).toString();
    system.shortName = object.value(QStringLiteral("short")).toString(object.value(QStringLiteral("shortName")).toString(system.name));
    system.year = object.value(QStringLiteral("year")).toString();
    system.manufacturer = object.value(QStringLiteral("manufacturer")).toString();
    system.blurb = object.value(QStringLiteral("blurb")).toString();
    system.accent = object.value(QStringLiteral("accent")).toString(QStringLiteral("#e6a23c"));
    system.secondary = object.value(QStringLiteral("secondary")).toString(QStringLiteral("#ffffff"));
    system.art = object.value(QStringLiteral("art")).toString(QStringLiteral("generic"));
    system.extensions = jsonStrings(object.value(QStringLiteral("extensions")));
    system.candidates = jsonStrings(object.value(QStringLiteral("candidates")));
    system.biosFlag = object.value(QStringLiteral("biosFlag")).toString();
    system.biosNote = object.value(QStringLiteral("biosNote")).toString();
    system.biosRequired = object.value(QStringLiteral("biosRequired")).toBool();
    system.flatpakId = object.value(QStringLiteral("flatpakId")).toString();
    system.installHint = object.value(QStringLiteral("installHint")).toString();
    system.builtin = builtin;
    if (object.value(QStringLiteral("plainArgs")).isArray())
        system.defaultPlainArgs = jsonStrings(object.value(QStringLiteral("plainArgs")));
    else
        system.plainOverride = object.value(QStringLiteral("plainArgs")).toString();
    if (object.value(QStringLiteral("gameArgs")).isArray())
        system.defaultGameArgs = jsonStrings(object.value(QStringLiteral("gameArgs")));
    else
        system.gameOverride = object.value(QStringLiteral("gameArgs")).toString();
    if (system.defaultGameArgs.isEmpty() && system.gameOverride.isEmpty())
        system.defaultGameArgs = {QStringLiteral("{file}")};
    const QJsonObject byExt = object.value(QStringLiteral("argsByExt")).toObject();
    for (const QString &key : byExt.keys())
        system.argsByExt.insert(key.toLower(), jsonStrings(byExt.value(key)));
    if (object.contains(QStringLiteral("command")))
        system.commandOverride = object.value(QStringLiteral("command")).toString();
    if (object.contains(QStringLiteral("bios"))) {
        system.biosPath = object.value(QStringLiteral("bios")).toString();
        system.biosUserSet = object.value(QStringLiteral("biosUserSet")).toBool(true);
    }
    return system;
}

Library::Game gameFromJson(const QJsonObject &object)
{
    Library::Game game;
    game.id = object.value(QStringLiteral("id")).toString();
    game.systemId = object.value(QStringLiteral("systemId")).toString();
    game.title = object.value(QStringLiteral("title")).toString();
    game.year = object.value(QStringLiteral("year")).toString();
    game.developer = object.value(QStringLiteral("developer")).toString();
    game.blurb = object.value(QStringLiteral("blurb")).toString();
    game.path = object.value(QStringLiteral("path")).toString();
    game.kind = object.value(QStringLiteral("kind")).toString();
    game.bundled = object.value(QStringLiteral("bundled")).toString();
    game.pathHint = object.value(QStringLiteral("pathHint")).toString();
    game.lastPlayed = object.value(QStringLiteral("lastPlayed")).toString();
    game.favorite = object.value(QStringLiteral("favorite")).toBool();
    if (game.kind.isEmpty())
        game.kind = QStringLiteral("classic");
    game.example = object.contains(QStringLiteral("example")) ? object.value(QStringLiteral("example")).toBool()
                                                             : game.kind != QLatin1String("user");
    return game;
}

QJsonObject gameToJson(const Library::Game &game)
{
    QJsonObject object;
    object.insert(QStringLiteral("id"), game.id);
    object.insert(QStringLiteral("systemId"), game.systemId);
    object.insert(QStringLiteral("title"), game.title);
    object.insert(QStringLiteral("year"), game.year);
    object.insert(QStringLiteral("developer"), game.developer);
    object.insert(QStringLiteral("blurb"), game.blurb);
    object.insert(QStringLiteral("path"), game.path);
    object.insert(QStringLiteral("kind"), game.kind);
    object.insert(QStringLiteral("bundled"), game.bundled);
    object.insert(QStringLiteral("pathHint"), game.pathHint);
    object.insert(QStringLiteral("lastPlayed"), game.lastPlayed);
    object.insert(QStringLiteral("favorite"), game.favorite);
    object.insert(QStringLiteral("example"), game.example);
    return object;
}

} // namespace

void Library::applyCatalog(const QJsonObject &catalog, const QJsonObject &saved)
{
    m_systems.clear();
    m_games.clear();
    m_hidden.clear();
    for (const QJsonValue &value : saved.value(QStringLiteral("hidden")).toArray())
        m_hidden << value.toString();

    const QJsonObject overrides = saved.value(QStringLiteral("overrides")).toObject();
    for (const QJsonValue &value : catalog.value(QStringLiteral("systems")).toArray()) {
        System system = systemFromJson(value.toObject(), true);
        const QJsonObject override = overrides.value(system.id).toObject();
        if (override.contains(QStringLiteral("command")))
            system.commandOverride = override.value(QStringLiteral("command")).toString();
        if (override.contains(QStringLiteral("plainArgs")))
            system.plainOverride = override.value(QStringLiteral("plainArgs")).toString();
        if (override.contains(QStringLiteral("gameArgs")))
            system.gameOverride = override.value(QStringLiteral("gameArgs")).toString();
        if (override.contains(QStringLiteral("extensions")))
            system.extensionsOverride = override.value(QStringLiteral("extensions")).toString();
        if (override.contains(QStringLiteral("bios"))) {
            system.biosPath = override.value(QStringLiteral("bios")).toString();
            system.biosUserSet = true;
        }
        m_systems.append(system);
    }
    for (const QJsonValue &value : saved.value(QStringLiteral("customSystems")).toArray())
        m_systems.append(systemFromJson(value.toObject(), false));

    QList<QString> seen;
    if (saved.contains(QStringLiteral("games"))) {
        for (const QJsonValue &value : saved.value(QStringLiteral("games")).toArray()) {
            Game game = gameFromJson(value.toObject());
            if (game.id.isEmpty())
                continue;
            m_games.append(game);
            seen.append(game.id);
        }
    }
    for (const QJsonValue &value : catalog.value(QStringLiteral("games")).toArray()) {
        Game game = gameFromJson(value.toObject());
        game.example = true;
        if (seen.contains(game.id) || m_hidden.contains(game.id))
            continue;
        m_games.append(game);
    }
}

void Library::load()
{
    QFile catalogFile(QStringLiteral(":/data/catalog.json"));
    if (!catalogFile.open(QIODevice::ReadOnly)) {
        setStatus(QStringLiteral("The built-in shelf catalog is missing."));
        return;
    }
    const QJsonObject catalog = QJsonDocument::fromJson(catalogFile.readAll()).object();
    QJsonObject saved;
    QFile saveFile(m_dataDir + QStringLiteral("/library.json"));
    if (saveFile.open(QIODevice::ReadOnly))
        saved = QJsonDocument::fromJson(saveFile.readAll()).object();

    applyCatalog(catalog, saved);
    bindLocalFiles();
    probeSystems();
    save();
    m_systemModel->refresh();
    notifyChanged();
}

void Library::save() const
{
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    QJsonArray hidden;
    for (const QString &id : m_hidden)
        hidden.append(id);
    root.insert(QStringLiteral("hidden"), hidden);

    QJsonObject overrides;
    QJsonArray custom;
    for (const System &system : m_systems) {
        if (!system.builtin) {
            QJsonObject object;
            object.insert(QStringLiteral("id"), system.id);
            object.insert(QStringLiteral("name"), system.name);
            object.insert(QStringLiteral("short"), system.shortName);
            object.insert(QStringLiteral("year"), system.year);
            object.insert(QStringLiteral("manufacturer"), system.manufacturer);
            object.insert(QStringLiteral("blurb"), system.blurb);
            object.insert(QStringLiteral("accent"), system.accent);
            object.insert(QStringLiteral("secondary"), system.secondary);
            object.insert(QStringLiteral("art"), system.art);
            QJsonArray extensions;
            for (const QString &ext : system.extensions)
                extensions.append(ext);
            object.insert(QStringLiteral("extensions"), extensions);
            object.insert(QStringLiteral("command"), system.commandOverride);
            object.insert(QStringLiteral("plainArgs"), system.plainOverride);
            object.insert(QStringLiteral("gameArgs"), system.gameOverride.isEmpty() ? QStringLiteral("{file}") : system.gameOverride);
            object.insert(QStringLiteral("bios"), system.biosPath);
            object.insert(QStringLiteral("biosUserSet"), system.biosUserSet);
            object.insert(QStringLiteral("biosFlag"), system.biosFlag);
            object.insert(QStringLiteral("biosNote"), system.biosNote);
            object.insert(QStringLiteral("biosRequired"), system.biosRequired);
            object.insert(QStringLiteral("builtin"), false);
            custom.append(object);
            continue;
        }
        QJsonObject override;
        if (!system.commandOverride.isEmpty())
            override.insert(QStringLiteral("command"), system.commandOverride);
        if (!system.plainOverride.isEmpty())
            override.insert(QStringLiteral("plainArgs"), system.plainOverride);
        if (!system.gameOverride.isEmpty())
            override.insert(QStringLiteral("gameArgs"), system.gameOverride);
        if (!system.extensionsOverride.isEmpty())
            override.insert(QStringLiteral("extensions"), system.extensionsOverride);
        if (system.biosUserSet)
            override.insert(QStringLiteral("bios"), system.biosPath);
        if (!override.isEmpty())
            overrides.insert(system.id, override);
    }
    root.insert(QStringLiteral("overrides"), overrides);
    root.insert(QStringLiteral("customSystems"), custom);

    QJsonArray games;
    for (const Game &game : m_games)
        games.append(gameToJson(game));
    root.insert(QStringLiteral("games"), games);

    QSaveFile out(m_dataDir + QStringLiteral("/library.json"));
    if (!out.open(QIODevice::WriteOnly))
        return;
    out.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    out.commit();
}

QVariantMap Library::gameMap(const Game &game) const
{
    const System *system = findSystem(game.systemId);
    QString error;
    const QStringList command = system ? fullCommand(*system, game.path, &error) : QStringList();
    QVariantMap map;
    map.insert(QStringLiteral("gameId"), game.id);
    map.insert(QStringLiteral("title"), game.title);
    map.insert(QStringLiteral("systemId"), game.systemId);
    map.insert(QStringLiteral("systemName"), system ? system->name : QString());
    map.insert(QStringLiteral("shortName"), system ? system->shortName : QString());
    map.insert(QStringLiteral("accent"), system ? system->accent : QStringLiteral("#e6a23c"));
    map.insert(QStringLiteral("year"), game.year);
    map.insert(QStringLiteral("developer"), game.developer);
    map.insert(QStringLiteral("blurb"), game.blurb);
    map.insert(QStringLiteral("path"), game.path);
    map.insert(QStringLiteral("fileReady"), fileReady(game));
    map.insert(QStringLiteral("playable"), playable(game));
    map.insert(QStringLiteral("favorite"), game.favorite);
    map.insert(QStringLiteral("example"), game.example);
    map.insert(QStringLiteral("kind"), game.kind);
    map.insert(QStringLiteral("lastPlayed"), prettyPlayed(game.lastPlayed));
    map.insert(QStringLiteral("blockReason"), blockReason(game));
    map.insert(QStringLiteral("command"), command.isEmpty() ? error : joinCommand(command));
    return map;
}

QVariantMap Library::systemMap(const System &system) const
{
    QVariantMap map;
    map.insert(QStringLiteral("id"), system.id);
    map.insert(QStringLiteral("name"), system.name);
    map.insert(QStringLiteral("shortName"), system.shortName);
    map.insert(QStringLiteral("year"), system.year);
    map.insert(QStringLiteral("blurb"), system.blurb);
    map.insert(QStringLiteral("accent"), system.accent);
    map.insert(QStringLiteral("builtin"), system.builtin);
    map.insert(QStringLiteral("extensions"), effectiveExtensions(system).join(QLatin1Char(' ')));
    map.insert(QStringLiteral("commandOverride"), system.commandOverride);
    map.insert(QStringLiteral("detectedCommand"), system.emulatorReady ? system.resolvedDisplay : QString());
    map.insert(QStringLiteral("ready"), system.emulatorReady);
    map.insert(QStringLiteral("plainArgs"), system.plainOverride);
    map.insert(QStringLiteral("plainDefault"), joinCommand(system.defaultPlainArgs));
    map.insert(QStringLiteral("gameArgs"), system.gameOverride);
    map.insert(QStringLiteral("gameDefault"), joinCommand(system.defaultGameArgs));
    map.insert(QStringLiteral("bios"), system.biosPath);
    map.insert(QStringLiteral("biosNote"), system.biosNote);
    map.insert(QStringLiteral("biosFlag"), system.biosFlag);
    map.insert(QStringLiteral("biosRequired"), system.biosRequired);
    map.insert(QStringLiteral("flatpakId"), system.flatpakId);
    map.insert(QStringLiteral("installHint"), system.installHint);
    map.insert(QStringLiteral("gameCount"), countGames(system.id, false));
    map.insert(QStringLiteral("readyCount"), countGames(system.id, true));
    return map;
}

QVariantMap Library::game(const QString &id) const
{
    const Game *found = findGame(id);
    return found ? gameMap(*found) : QVariantMap();
}

QVariantMap Library::system(const QString &id) const
{
    const System *found = findSystem(id);
    return found ? systemMap(*found) : QVariantMap();
}

QString Library::systemIdAt(int row) const
{
    if (row < 0 || row >= m_systems.size())
        return {};
    return m_systems.at(row).id;
}

bool Library::systemReady(const QString &id) const
{
    const System *system = findSystem(id);
    return system && system->emulatorReady;
}

QVariantList Library::playableGames() const
{
    QVariantList list;
    for (const Game &game : m_games) {
        if (!playable(game))
            continue;
        list.append(gameMap(game));
        if (list.size() >= 12)
            break;
    }
    return list;
}

QVariantList Library::classicGames() const
{
    QVariantList list;
    for (const Game &game : m_games) {
        if (game.kind != QLatin1String("classic") || fileReady(game))
            continue;
        list.append(gameMap(game));
    }
    return list;
}

QString Library::commandForGame(const QString &id) const
{
    const Game *game = findGame(id);
    if (!game)
        return {};
    const System *system = findSystem(game->systemId);
    if (!system)
        return {};
    QString error;
    const QStringList command = fullCommand(*system, game->path, &error);
    return command.isEmpty() ? error : joinCommand(command);
}

QString Library::commandForSystem(const QString &id) const
{
    const System *system = findSystem(id);
    if (!system)
        return {};
    QString error;
    const QStringList command = fullCommand(*system, QString(), &error);
    return command.isEmpty() ? error : joinCommand(command);
}

QString Library::localPath(const QVariant &value) const
{
    return variantPath(value);
}

void Library::addFiles(const QString &systemId, const QVariantList &urls)
{
    int added = 0;
    int linked = 0;
    int skipped = 0;
    int unknown = 0;
    for (const QVariant &url : urls) {
        const QString path = variantPath(url);
        if (path.isEmpty())
            continue;
        const QFileInfo info(path);
        if (info.isDir()) {
            QDirIterator it(path, QDir::Files, QDirIterator::Subdirectories);
            int guard = 0;
            while (it.hasNext() && guard < 2000) {
                it.next();
                ++guard;
                const int result = ingestFile(systemId, it.filePath(), !systemId.isEmpty(), &linked, &skipped);
                if (result == 1)
                    ++added;
                else if (result < 0)
                    ++unknown;
            }
            continue;
        }
        const int result = ingestFile(systemId, path, !systemId.isEmpty(), &linked, &skipped);
        if (result == 1)
            ++added;
        else if (result < 0)
            ++unknown;
    }
    save();
    notifyChanged();
    QStringList parts;
    if (added)
        parts << QStringLiteral("Added %1").arg(added);
    if (linked)
        parts << QStringLiteral("linked %1 classic").arg(linked);
    if (skipped)
        parts << QStringLiteral("skipped %1 already on the shelf").arg(skipped);
    if (unknown)
        parts << QStringLiteral("%1 file type not recognized").arg(unknown);
    setStatus(parts.isEmpty() ? QStringLiteral("Nothing new to add.") : parts.join(QStringLiteral(", ")) + QLatin1Char('.'));
}

void Library::scanDirectory(const QString &systemId, const QVariant &folder, bool recursive)
{
    const QString path = variantPath(folder);
    if (path.isEmpty() || !QFileInfo(path).isDir()) {
        setStatus(QStringLiteral("That folder is not there."));
        return;
    }
    addFiles(systemId, {QVariant(path)});
    Q_UNUSED(recursive);
}

void Library::launchGame(const QString &id)
{
    Game *game = findGame(id);
    if (!game)
        return;
    const System *system = findSystem(game->systemId);
    if (!system)
        return;
    if (!playable(*game)) {
        setStatus(blockReason(*game));
        return;
    }
    QString error;
    const QStringList command = fullCommand(*system, game->path, &error);
    if (command.isEmpty()) {
        setStatus(error);
        return;
    }
    if (!startCommand(command, QFileInfo(game->path).absolutePath(), &error)) {
        setStatus(error);
        return;
    }
    game->lastPlayed = QDateTime::currentDateTime().toString(Qt::ISODate);
    save();
    notifyChanged();
    QString message = QStringLiteral("Playing %1.").arg(game->title);
    if (system->biosRequired && system->biosPath.isEmpty())
        message += QStringLiteral(" No Kickstart is set, so the emulator may ask for one.");
    setStatus(message);
}

void Library::launchSystem(const QString &id)
{
    const System *system = findSystem(id);
    if (!system) {
        setStatus(QStringLiteral("That machine is not on the shelf."));
        return;
    }
    QString error;
    const QStringList command = fullCommand(*system, QString(), &error);
    if (command.isEmpty()) {
        setStatus(error);
        return;
    }
    if (!startCommand(command, QDir::homePath(), &error)) {
        setStatus(error);
        return;
    }
    QString message = QStringLiteral("Powered on %1.").arg(system->name);
    if (system->biosRequired && system->biosPath.isEmpty())
        message += QStringLiteral(" Add a Kickstart ROM in Settings if the screen stays empty.");
    setStatus(message);
}

void Library::removeGame(const QString &id)
{
    for (int i = 0; i < m_games.size(); ++i) {
        if (m_games.at(i).id != id)
            continue;
        if (m_games.at(i).example && !m_hidden.contains(id))
            m_hidden.append(id);
        m_games.removeAt(i);
        save();
        notifyChanged();
        setStatus(QStringLiteral("Removed from the shelf. Restore examples in Settings if you want the card back."));
        return;
    }
}

void Library::toggleFavorite(const QString &id)
{
    Game *game = findGame(id);
    if (!game)
        return;
    game->favorite = !game->favorite;
    save();
    notifyChanged();
    setStatus(game->favorite ? QStringLiteral("Starred %1.").arg(game->title) : QStringLiteral("Removed the star."));
}

void Library::setTitle(const QString &id, const QString &title)
{
    Game *game = findGame(id);
    if (!game)
        return;
    const QString next = title.trimmed();
    if (next.isEmpty() || next == game->title)
        return;
    game->title = next;
    save();
    notifyChanged();
}

void Library::setGamePath(const QString &id, const QVariant &pathValue)
{
    Game *game = findGame(id);
    if (!game)
        return;
    const QString path = variantPath(pathValue).trimmed();
    if (!path.isEmpty() && !QFileInfo::exists(path)) {
        setStatus(QStringLiteral("That file is not there."));
        return;
    }
    game->path = path.isEmpty() ? QString() : QFileInfo(path).absoluteFilePath();
    save();
    notifyChanged();
    setStatus(path.isEmpty() ? QStringLiteral("Cleared the disk image.") : QStringLiteral("Disk image set for %1.").arg(game->title));
}

void Library::updateSystem(const QString &id, const QString &command, const QString &gameArgs, const QString &plainArgs,
                           const QString &bios, const QString &extensions)
{
    System *system = findSystem(id);
    if (!system)
        return;
    system->commandOverride = command.trimmed();
    system->gameOverride = gameArgs.trimmed();
    system->plainOverride = plainArgs.trimmed();
    system->extensionsOverride = extensions.trimmed();
    system->biosPath = bios.trimmed();
    system->biosUserSet = true;
    probeSystems();
    save();
    notifyChanged();
    setStatus(system->emulatorReady ? QStringLiteral("Saved %1.").arg(system->name)
                                    : QStringLiteral("Saved %1, but the emulator command was not found.").arg(system->name));
}

void Library::addSystem(const QString &name, const QString &shortName, const QString &extensions, const QString &command,
                        const QString &accent)
{
    const QString clean = name.trimmed();
    if (clean.isEmpty()) {
        setStatus(QStringLiteral("A machine needs a name."));
        return;
    }
    System system;
    system.id = QStringLiteral("sys-") + QUuid::createUuid().toString(QUuid::Id128).left(8);
    system.name = clean;
    system.shortName = shortName.trimmed().isEmpty() ? clean.left(10) : shortName.trimmed();
    system.art = QStringLiteral("generic");
    system.accent = accent.isEmpty() ? QStringLiteral("#e6a23c") : accent;
    system.secondary = QStringLiteral("#ffffff");
    system.blurb = QStringLiteral("Added in RETRO Center.");
    system.builtin = false;
    system.commandOverride = command.trimmed();
    system.defaultGameArgs = {QStringLiteral("{file}")};
    for (QString ext : extensions.split(QRegularExpression(QStringLiteral("[,\\s]+")), Qt::SkipEmptyParts)) {
        ext = ext.toLower();
        if (!ext.startsWith(QLatin1Char('.')))
            ext.prepend(QLatin1Char('.'));
        system.extensions << ext;
    }
    if (system.extensions.isEmpty())
        system.extensions << QStringLiteral(".*");
    m_systems.append(system);
    m_systemModel->refresh();
    probeSystems();
    save();
    notifyChanged();
    setStatus(QStringLiteral("Added %1.").arg(system.name));
}

void Library::removeSystem(const QString &id)
{
    const int row = systemRow(id);
    if (row < 0)
        return;
    if (m_systems.at(row).builtin) {
        setStatus(QStringLiteral("The built-in machines stay. Clear the command if you want a different emulator."));
        return;
    }
    const QString name = m_systems.at(row).name;
    for (int i = m_games.size() - 1; i >= 0; --i) {
        if (m_games.at(i).systemId != id)
            continue;
        if (m_games.at(i).example)
            m_hidden.append(m_games.at(i).id);
        m_games.removeAt(i);
    }
    m_systems.removeAt(row);
    if (m_systemFilter == id)
        m_systemFilter.clear();
    m_systemModel->refresh();
    save();
    notifyChanged();
    setStatus(QStringLiteral("Removed %1.").arg(name));
}

void Library::installFlatpak(const QString &id)
{
    const System *system = findSystem(id);
    if (!system || system->flatpakId.isEmpty()) {
        setStatus(QStringLiteral("There is no Flatpak package for this machine."));
        return;
    }
    if (m_busy) {
        setStatus(QStringLiteral("An install is already running."));
        return;
    }
    const QString flatpak = findProgram(QStringLiteral("flatpak"));
    if (flatpak.isEmpty()) {
        setStatus(QStringLiteral("Flatpak is not installed."));
        return;
    }
    m_busyLog.clear();
    emit busyLogChanged();
    setBusy(true);
    auto *process = new QProcess(this);
    m_install = process;
    process->setProgram(flatpak);
    process->setArguments({QStringLiteral("install"), QStringLiteral("-y"), QStringLiteral("--user"),
                           QStringLiteral("--noninteractive"), QStringLiteral("flathub"), system->flatpakId});
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyRead, this, [this, process] {
        m_busyLog += QString::fromUtf8(process->readAll());
        if (m_busyLog.size() > 6000)
            m_busyLog = m_busyLog.right(6000);
        emit busyLogChanged();
    });
    connect(process, &QProcess::finished, this, [this, process](int code) {
        setBusy(false);
        probeSystems();
        notifyChanged();
        setStatus(code == 0 ? QStringLiteral("Install finished.") : QStringLiteral("Install failed. The log is in Settings."));
        if (m_install == process)
            m_install = nullptr;
        process->deleteLater();
    });
    process->start();
    setStatus(QStringLiteral("Installing %1…").arg(system->flatpakId));
}

void Library::probe()
{
    probeSystems();
    notifyChanged();
    setStatus(QStringLiteral("Looked for emulators again."));
}

void Library::openDataFolder() const
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(m_dataDir));
}

void Library::revealPath(const QString &path) const
{
    const QFileInfo info(path);
    const QString dir = info.exists() ? (info.isDir() ? info.absoluteFilePath() : info.absolutePath()) : m_dataDir;
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir));
}

void Library::copyText(const QString &text)
{
    if (auto *clipboard = QGuiApplication::clipboard())
        clipboard->setText(text);
    setStatus(QStringLiteral("Copied the launch command."));
}

void Library::restoreExamples()
{
    m_hidden.clear();
    QFile catalogFile(QStringLiteral(":/data/catalog.json"));
    if (!catalogFile.open(QIODevice::ReadOnly))
        return;
    const QJsonObject catalog = QJsonDocument::fromJson(catalogFile.readAll()).object();
    QList<QString> seen;
    for (const Game &game : m_games)
        seen.append(game.id);
    for (const QJsonValue &value : catalog.value(QStringLiteral("games")).toArray()) {
        Game game = gameFromJson(value.toObject());
        game.example = true;
        if (seen.contains(game.id))
            continue;
        m_games.append(game);
    }
    bindLocalFiles();
    save();
    notifyChanged();
    setStatus(QStringLiteral("Examples are back on the shelf."));
}

int Library::selfCheck() const
{
    QTextStream out(stdout);
    out << "RETRO Center check\n";
    out << "data: " << m_dataDir << "\n";
    int problems = 0;
    if (m_systems.size() < 5) {
        out << "FAIL expected 5 built-in machines, found " << m_systems.size() << "\n";
        ++problems;
    }
    const QStringList required = {QStringLiteral("c64"), QStringLiteral("apple2"), QStringLiteral("ti99"),
                                  QStringLiteral("amiga"), QStringLiteral("atarist")};
    for (const QString &id : required) {
        const System *system = findSystem(id);
        if (!system) {
            out << "FAIL missing machine " << id << "\n";
            ++problems;
            continue;
        }
        out << (system->emulatorReady ? "OK   " : "MISS ") << system->shortName << "  " << system->resolvedDisplay << "\n";
        if (!system->emulatorReady)
            ++problems;
    }
    int playable = 0;
    for (const Game &game : m_games) {
        if (this->playable(game))
            ++playable;
        if (game.bundled.isEmpty())
            continue;
        const bool ok = fileReady(game) && QFileInfo(game.path).size() > 32;
        out << (ok ? "OK   demo " : "FAIL demo ") << game.title << "  " << game.path << "\n";
        if (!ok)
            ++problems;
    }
    out << "games: " << m_games.size() << "  playable: " << playable << "\n";
    if (m_games.size() < 30) {
        out << "FAIL shelf is too small\n";
        ++problems;
    }
    out << (problems ? "RESULT fail\n" : "RESULT ok\n");
    return problems ? 1 : 0;
}

void Library::writeDesktopEntry() const
{
#ifndef Q_OS_LINUX
    return;
#else
    const QString apps = QDir::homePath() + QStringLiteral("/.local/share/applications");
    QDir().mkpath(apps);
    QFile iconIn(QStringLiteral(":/assets/retrocenter.svg"));
    const QString iconPath = m_dataDir + QStringLiteral("/retrocenter.svg");
    if (iconIn.open(QIODevice::ReadOnly)) {
        QFile iconOut(iconPath);
        if (iconOut.open(QIODevice::WriteOnly))
            iconOut.write(iconIn.readAll());
    }
    const QString exec = QCoreApplication::applicationFilePath();
    QFile desktop(apps + QStringLiteral("/retro-center.desktop"));
    if (!desktop.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return;
    const QString body = QStringLiteral(
                             "[Desktop Entry]\n"
                             "Type=Application\n"
                             "Version=1.0\n"
                             "Name=RETRO Center\n"
                             "Comment=Launch and organize C64, Apple II, TI-99/4A, Amiga, and Atari ST\n"
                             "Exec=\"%1\"\n"
                             "Icon=%2\n"
                             "Terminal=false\n"
                             "Categories=Game;\n"
                             "StartupWMClass=retro-center\n")
                             .arg(exec, iconPath);
    desktop.write(body.toUtf8());
#endif
}
