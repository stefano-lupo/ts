#include ".\headers\Navigator.h"

void Navigator::setup() {
  setTrackingConfig(1);
}

void Navigator::nextTrackingConfig() {
  disableNavigation();
  int nextTrackingConfig = this->trackingConfig * 2;
  if (nextTrackingConfig > 4) {
    nextTrackingConfig = 1;
  }

  setTrackingConfig(nextTrackingConfig);
}

void Navigator::setTrackingConfig(int trackingConfig) {
  Serial.print("Updaing tracking config to: ");
  Serial.println(trackingConfig);
  this->trackingConfig = trackingConfig;
  if (trackingConfig == 1) {
    motorController.setFullStep();
    tracker.setMotorTickPeriodMillis(3000);
  } else if (trackingConfig == 2) {
    motorController.setHalfStep();
    tracker.setMotorTickPeriodMillis(1500);
  } else {
    motorController.setQuarterStep();
    tracker.setMotorTickPeriodMillis(750);
  }
}

void Navigator::moveIfNesc() {
  // if (cooldownCounter > 0) {
  //   Serial.println("Cooling down");
  //   cooldownCounter--;
  //   return;
  // }

  switch (state) {
    case NavigatorState::TRACKING: {
      int movesNeeded = tracker.getMovesNeeded();
      if (movesNeeded > 0) {
        motorController.setDirection(MotorController::RA);
        motorController.stepMotor(movesNeeded);
        tracker.consumeMovesNeeded(movesNeeded);
        Serial.print("Tracking: Moved ");
        Serial.println(movesNeeded);
      }

      break;
    }

    /* This is fairly brittle at the moment */
    case NavigatorState::SLEWING: {
      Serial.print("Had target of ");
      char str2[16];
      targetCoord.formatString(str2);
      Serial.println(str2);

      Serial.print("Had current of ");
      char str3[16];
      currentCoord.formatString(str3);
      Serial.println(str3);

      Coordinate delta = Coordinate::getMinimumDelta(currentCoord, targetCoord);
      
      Serial.print("Had delta of ");
      char str[16];
      delta.formatString(str);
      Serial.println(str);

      if (delta.hours != 0) {
        int sign = delta.hours > 0 ? 1 : - 1;
        int hoursToSlewThisTick = min(abs(delta.hours), Navigator::MAX_HOURS_SLEW_PER_TICK) * sign;
        slewHours(hoursToSlewThisTick);

        // Only start the cooldown once finished slewing hours
        if (hoursToSlewThisTick == delta.hours) {
          startCooldown();
        }

        Serial.print("Slewed ");
        Serial.print(hoursToSlewThisTick);
        Serial.println(" hours");
      } else if (delta.minutes != 0) {
        slewMinutes(delta.minutes);
        Serial.print("Slewed ");
        Serial.print(delta.minutes);
        Serial.println(" minutes");
        startCooldown();
      } else {
        slewSeconds(delta.seconds);
        Serial.print("Slewed ");
        Serial.print(delta.seconds);
        Serial.println(" seconds");
        trackTarget();
        startCooldown();
      }


      break;
    }

    case NavigatorState::IDLE: {
      break;
    }

    default: {
      Serial.println("Navigator in unknown navigation state");
    }
  }
}

void Navigator::setTargetCoord(const Coordinate& coord) {
  targetCoord = coord;
  // char str[16];
  // targetCoord.formatString(str);
  // Serial.print("Setting target: ");
  // Serial.println(str);
}

void Navigator::setCurrentCoord(const Coordinate& coord) {
  currentCoord = coord;
}

const Coordinate& Navigator::getTargetCoord() {
  return targetCoord;
}

const Coordinate& Navigator::getCurrentCoord() {
  return currentCoord;
}

void Navigator::slewToTarget() {
  state = NavigatorState::SLEWING;
  tracker.startTracker();
}

void Navigator::trackTarget() {
  state = NavigatorState::TRACKING;
  tracker.startTracker();
}

void Navigator::disableNavigation() {
  state = NavigatorState::IDLE;
  tracker.stopTracker();
  motorController.disableMotor();
}

void Navigator::slewHours(int numHours) {
  boolean direction = getDirectionFromDelta(numHours);
  if (useFullSteps) {
    motorController.setFullStep();
    slew(direction, numHours * NUM_FULL_STEPS_IN_HOUR);
  } else {
    motorController.setHalfStep();
    slew(direction, numHours * NUM_HALF_STEPS_IN_HOUR);
  }
  currentCoord.addHours(numHours);
}

void Navigator::slewMinutes(int numMinutes) {
  boolean direction = getDirectionFromDelta(numMinutes);
  // motorController.setFullStep();
  // slew(direction, numMinutes * NUM_FULL_STEPS_IN_MINUTE);
    motorController.setHalfStep();
  slew(direction, numMinutes * NUM_HALF_STEPS_IN_MINUTE);

  currentCoord.addMinutes(numMinutes);
}

void Navigator::slewSeconds(int deltaSeconds) {
  boolean direction = getDirectionFromDelta(deltaSeconds);

  // int numFullSteps = abs(deltaSeconds) / SECONDS_PER_FULL_STEP;
  // motorController.setFullStep();
  // slew(direction, numFullSteps);

  int numHalfSteps = abs(deltaSeconds) / (SECONDS_PER_FULL_STEP / 2);
  motorController.setHalfStep();
  slew(direction, numHalfSteps);

  // Within 3 seconds at this point
  // byte remainingSeconds = abs(deltaSeconds) - numFullSteps * SECONDS_PER_FULL_STEP;
  byte remainingSeconds = abs(deltaSeconds) - numHalfSteps * (SECONDS_PER_FULL_STEP / 2);
  byte numQuarterSteps = remainingSeconds / (SECONDS_PER_FULL_STEP / 4);
  motorController.setQuarterStep();
  slew(direction, numQuarterSteps);

  currentCoord.addSeconds(deltaSeconds);
}

void Navigator::slew(boolean direction, int numSteps) {
  motorController.setDirection(direction);
  motorController.stepMotor(abs(numSteps));
}

boolean Navigator::getDirectionFromDelta(int deltaValue) {
  return deltaValue > 0 ? MotorController::RA : MotorController::ANTI_RA;
}

void Navigator::startCooldown() {
  Serial.println("Starting cooldown");
  // cooldownCounter = DEFAULT_COOLDOWN_VALUE;
}

char Navigator::getEncodedNavigationState() {
  switch (state) {
    case NavigatorState::IDLE:
      return 'I';
    case NavigatorState::TRACKING:
      return 'T';
    case NavigatorState::SLEWING:
      return 'S';
  }
}

char Navigator::getEncodedTrackingConfig() {
  return 'x';
}
