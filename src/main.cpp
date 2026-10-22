#include "library.h"

#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QPalette>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QScreen>

static QString pickFamily(const QStringList &families, const QStringList &preferred, const QString &fallback)
{
    for (const QString &name : preferred) {
        if (families.contains(name))
            return name;
    }
    return fallback;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("retro-center"));
    QCoreApplication::setApplicationVersion(QStringLiteral("1.0.0"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("RETRO Center"));

    const QStringList families = QFontDatabase::families();
    const QString sans = pickFamily(families, {QStringLiteral("Ubuntu Sans"), QStringLiteral("Fira Sans"),
                                                QStringLiteral("Noto Sans"), QStringLiteral("DejaVu Sans")},
                                    QStringLiteral("Sans Serif"));
    const QString mono = pickFamily(families, {QStringLiteral("Ubuntu Mono"), QStringLiteral("Fira Mono"),
                                                QStringLiteral("DejaVu Sans Mono"), QStringLiteral("Noto Sans Mono")},
                                    QStringLiteral("Monospace"));
    QFont font(sans);
    font.setPixelSize(15);
    app.setFont(font);

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(QStringLiteral("#12130f")));
    palette.setColor(QPalette::WindowText, QColor(QStringLiteral("#f4f1e6")));
    palette.setColor(QPalette::Base, QColor(QStringLiteral("#10120d")));
    palette.setColor(QPalette::AlternateBase, QColor(QStringLiteral("#1c1e18")));
    palette.setColor(QPalette::Text, QColor(QStringLiteral("#f4f1e6")));
    palette.setColor(QPalette::Button, QColor(QStringLiteral("#26291f")));
    palette.setColor(QPalette::ButtonText, QColor(QStringLiteral("#f4f1e6")));
    palette.setColor(QPalette::Highlight, QColor(QStringLiteral("#e6a23c")));
    palette.setColor(QPalette::HighlightedText, QColor(QStringLiteral("#1a1408")));
    palette.setColor(QPalette::PlaceholderText, QColor(QStringLiteral("#6f6b5e")));
    palette.setColor(QPalette::ToolTipBase, QColor(QStringLiteral("#1c1e18")));
    palette.setColor(QPalette::ToolTipText, QColor(QStringLiteral("#f4f1e6")));
    app.setPalette(palette);

    Library library;
    if (app.arguments().contains(QStringLiteral("--check")))
        return library.selfCheck();

    library.writeDesktopEntry();
    QQuickStyle::setStyle(QStringLiteral("Basic"));

    QQmlApplicationEngine engine;
    engine.addImportPath(QStringLiteral("qrc:/qml"));
    engine.rootContext()->setContextProperty(QStringLiteral("library"), &library);
    engine.rootContext()->setContextProperty(QStringLiteral("monoFont"), mono);
    engine.load(QUrl(QStringLiteral("qrc:/qml/Main.qml")));
    if (engine.rootObjects().isEmpty())
        return 1;

    QObject *root = engine.rootObjects().constFirst();
    if (auto *window = qobject_cast<QQuickWindow *>(root)) {
        if (QScreen *screen = window->screen()) {
            const QRect avail = screen->availableGeometry();
            if (window->width() > avail.width() || window->height() > avail.height())
                window->showMaximized();
        }
    }
    return app.exec();
}
