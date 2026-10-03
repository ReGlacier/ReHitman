import {Button, Card, Icon, Text} from '@gravity-ui/uikit';
import {CircleCheck, PlugWire} from '@gravity-ui/icons';
import type {AgentInfo} from '../api';

export type KickState = 'pending' | 'kicked' | 'gone';

export interface AgentCardProps {
  agent: AgentInfo;
  kick?: KickState;
  ghost?: boolean;
  onDisconnect: (agent: AgentInfo) => void;
}

function ageLabel(seconds: number): string {
  if (seconds < 2) {
    return 'только что';
  }
  if (seconds < 60) {
    return `${Math.round(seconds)} с назад`;
  }
  const minutes = Math.floor(seconds / 60);
  if (minutes < 60) {
    return `${minutes} мин назад`;
  }
  return `${Math.floor(minutes / 60)} ч назад`;
}

function buttonLabel(kick?: KickState): string {
  switch (kick) {
    case 'pending':
      return 'Отключение…';
    case 'kicked':
      return 'Отключён';
    case 'gone':
      return 'Уже отключён';
    default:
      return 'Отключить';
  }
}

export function AgentCard({agent, kick, ghost, onDisconnect}: AgentCardProps) {
  const done = kick === 'kicked' || kick === 'gone';
  const isGhost = Boolean(ghost) || done;

  return (
    <Card
      type="container"
      view="outlined"
      className={`hyper-agent${isGhost ? ' hyper-agent--ghost' : ''}`}
    >
      <div className="hyper-agent__name">
        <span className="hyper-agent__dot" aria-hidden="true" />
        {agent.name}
      </div>

      <Text as="div" variant="code-1" color="secondary" className="hyper-mono">
        {agent.database}
      </Text>
      <Text as="div" variant="caption-2" color="hint">
        онлайн · {ageLabel(agent.ageSeconds)}
      </Text>
      <Text as="div" variant="caption-2" color="hint" className="hyper-mono">
        id: {agent.id}
      </Text>

      <div className="hyper-agent__footer">
        <Button
          className={done ? undefined : 'hyper-kick'}
          view={done ? 'outlined-success' : 'outlined-danger'}
          width="max"
          size="m"
          disabled={Boolean(kick)}
          loading={kick === 'pending'}
          onClick={() => onDisconnect(agent)}
        >
          <Button.Icon side="start">
            <Icon data={done ? CircleCheck : PlugWire} />
          </Button.Icon>
          {buttonLabel(kick)}
        </Button>
      </div>
    </Card>
  );
}
