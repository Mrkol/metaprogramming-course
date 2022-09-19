
# Base stage that can be used for development
FROM alpine:edge AS metacourse-base

ENV TZ=Europe/Moscow

ENV LANG C.UTF-8
ENV LC_ALL C.UTF-8


RUN apk update && apk upgrade
RUN apk add --no-cache git build-base clang make cmake lld compiler-rt compiler-rt-static zsh



FROM metacourse-base AS metacourse-tester

RUN adduser --disabled-password --shell /bin/zsh tester
COPY .ssh /home/tester/.ssh
RUN chown -R tester /home/tester/.ssh
COPY run_tests.sh /home/tester
RUN chown tester /home/tester/run_tests.sh
RUN chmod +x /home/tester/run_tests.sh

USER tester
WORKDIR /home/tester

ENTRYPOINT [ "/home/tester/run_tests.sh" ]


FROM metacourse-base AS metacourse-dev

RUN apk add --no-cache libuser openssh sudo

ENV USER_ID=65535
ENV GROUP_ID=65535
ENV USER_NAME=student
ENV GROUP_NAME=students

RUN addgroup -g $GROUP_ID $GROUP_NAME \
    && adduser --shell /bin/zsh --disabled-password --gecos "" \
      --uid $USER_ID --ingroup $GROUP_NAME $USER_NAME
RUN echo "$USER_NAME:123" | chpasswd
RUN echo "$USER_NAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

USER $USER_NAME

RUN wget https://github.com/robbyrussell/oh-my-zsh/raw/master/tools/install.sh -O - | zsh || true
ENV ZSH_THEME bira
