all: onboard supervisor ats

ONBOARD_SSH_IPS = 192.168.1.151 192.168.1.173 192.168.1.172

SUPERVISOR_SSH_IP = 192.168.1.159

SSH_USER = pi
SSH_PASS = raspberry
SSH_PATH = /home/pi/Unirail-25

SUPERVISOR_RUN_PORT = 3000
ONBOARD_RUN_PORT = 3000

DEFAULT_MISSION = 3

onboard:
	$(MAKE) -C app/Onboard

supervisor:
	$(MAKE) -C app/Supervisor

ats:
	$(MAKE) -C app/ATS

clean:
	$(MAKE) -C app/Onboard clean
	$(MAKE) -C app/Supervisor clean
	$(MAKE) -C app/ATS clean

install: install-onboard-1 install-onboard-2 install-onboard-3 install-supervisor

install-supervisor:
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(SUPERVISOR_SSH_IP) "rm -rf $(SSH_PATH)/app"
	sshpass -p $(SSH_PASS) scp -r ./app $(SSH_USER)@$(SUPERVISOR_SSH_IP):$(SSH_PATH)
	sshpass -p $(SSH_PASS) scp -r ./Makefile $(SSH_USER)@$(SUPERVISOR_SSH_IP):$(SSH_PATH)
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(SUPERVISOR_SSH_IP) "export TERM=xterm; cd $(SSH_PATH) && make clean && make supervisor && clear"

install-onboard-%:
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)) "rm -rf $(SSH_PATH)/app"
	sshpass -p $(SSH_PASS) scp -r ./app $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)):$(SSH_PATH)
	sshpass -p $(SSH_PASS) scp -r ./Makefile $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)):$(SSH_PATH)
	sshpass -p $(SSH_PASS) ssh $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)) "export TERM=xterm; cd $(SSH_PATH) && make clean && make onboard && clear"

run-supervisor: install-supervisor
	sshpass -p $(SSH_PASS) ssh -tt $(SSH_USER)@$(SUPERVISOR_SSH_IP) "cd $(SSH_PATH) && ./app/Supervisor/bin/supervisor $(SUPERVISOR_RUN_PORT)"

run-onboard-%: install-onboard-%
	sshpass -p $(SSH_PASS) ssh -tt $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)) "cd $(SSH_PATH) && ./app/Onboard/bin/onboard $* $(DEFAULT_MISSION) $(SUPERVISOR_SSH_IP) $(SUPERVISOR_RUN_PORT) $(ONBOARD_RUN_PORT)"

run-ats: ats
	./app/ATS/bin/ats $(ONBOARD_RUN_PORT)

test-supervisor: install-supervisor
	sshpass -p $(SSH_PASS) ssh -tt $(SSH_USER)@$(SUPERVISOR_SSH_IP) "cd $(SSH_PATH)/app/Supervisor && make test"

test-onboard-%: install-onboard-%
	sshpass -p $(SSH_PASS) ssh -tt $(SSH_USER)@$(word $*, $(ONBOARD_SSH_IPS)) "cd $(SSH_PATH)/app/Onboard && make test"