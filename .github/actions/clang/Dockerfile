FROM base/devel:latest

LABEL "name"="cmake clang build"
LABEL "maintainer"="ashley towns"
LABEL "version"="1.0.0"

LABEL "com.github.actions.icon"="code"
LABEL "com.github.actions.color"="green-dark"
LABEL "com.github.actions.name"="Clang Tools"
LABEL "com.github.actions.description"="This is an Action to run clang and cmake commands."

RUN pacman -Syu --noconfirm 
RUN pacman -S cmake clang --noconfirm

COPY entrypoint.sh /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
