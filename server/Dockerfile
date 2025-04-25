FROM ubuntu
RUN mkdir -p /srv
WORKDIR /srv
#move exordium/bin/exordium to /srv
COPY ./exordium/bin/exordium /srv
#RUN apt update && apt upgrade -y

CMD ["./exordium"]
