.PHONY: clean All

All:
	@echo "----------Building project:[ mir_core - Release ]----------"
	@cd "/var/www/miranda-ng/src/mir_core" && "$(MAKE)" -f  "mir_core.mk"
	@echo "----------Building project:[ mir_app - Release ]----------"
	@cd "/var/www/miranda-ng/src/mir_app" && "$(MAKE)" -f  "mir_app.mk"
clean:
	@echo "----------Cleaning project:[ mir_core - Release ]----------"
	@cd "/var/www/miranda-ng/src/mir_core" && "$(MAKE)" -f  "mir_core.mk"  clean
	@echo "----------Cleaning project:[ mir_app - Release ]----------"
	@cd "/var/www/miranda-ng/src/mir_app" && "$(MAKE)" -f  "mir_app.mk" clean
